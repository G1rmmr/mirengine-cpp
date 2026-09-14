#include "ScriptSystem.hpp"
#include "../core/Manager.hpp"
#include "../component/Transform.hpp"
#include "../component/Sprite.hpp"
#include "../component/Rigidbody.hpp"
#include "../component/Collider.hpp"
#include "../component/Tag.hpp"
#include "../device/Input.hpp"
#include "../device/Window.hpp"
#include "../device/Key.hpp"
#include "../system/Movement.hpp"
#include "../system/Collision.hpp"
#include "../system/Hierarchy.hpp"
#include "../system/Event.hpp"
#include "../asset/Animation.hpp"
#include "../asset/Resource.hpp"
#include "../asset/Scene.hpp"
#include "../asset/Sound.hpp"
#include "../asset/Texture.hpp"
#include "../asset/Font.hpp"
#include "../util/Debugger.hpp"
#include "../util/Timer.hpp"
#include "../util/Profiler.hpp"
#include "../device/Shader.hpp"
#include "../device/GPUPipeline.hpp"
#include "../device/GPUDevice.hpp"
#include "../view/Border.hpp"
#include "../view/Button.hpp"
#include "../view/Label.hpp"
#include "../view/Camera.hpp"
#include "../math/Math.hpp"
#include <container/List.hpp>
#include <container/FixedGraph.hpp>

#include <array>
#include <iostream>
#include <optional>
#include <span>
#include <string>

namespace mir::script {
	struct ScriptScheduler {
		struct Dependency {};
		struct Record {
			sol::protected_function Callback;
		};
		struct Listener {
			String<> Name;
			sol::protected_function Callback;
			bool Active = false;
		};

		using Graph = FixedGraph<Record, Dependency, config::MAX_SYSTEM, config::MAX_SYSTEM_DEPENDENCIES>;
		static constexpr std::size_t MAX_LISTENERS = event::MAX_EVENT_TYPES * event::MAX_CALLBACKS_PER_EVENT;

		std::optional<Graph> Simulation{std::in_place};
		std::optional<Graph> PostCommit{std::in_place};
		Graph::TraversalScratch SimulationScratch{};
		Graph::TraversalScratch PostCommitScratch{};
		std::array<Listener, MAX_LISTENERS> Listeners{};

		[[nodiscard]] Graph& Select(const core::SystemPhase phase) noexcept {
			return phase == core::SystemPhase::Simulation ? *Simulation : *PostCommit;
		}

		[[nodiscard]] Graph::TraversalScratch& SelectScratch(const core::SystemPhase phase) noexcept {
			return phase == core::SystemPhase::Simulation ? SimulationScratch : PostCommitScratch;
		}

		[[nodiscard]] ScriptSystemId Register(sol::protected_function callback, const core::SystemPhase phase) {
			if (!callback.valid()) return {};
			Graph& graph = Select(phase);
			const PoolHandle handle = graph.TryAddVertex(Record{std::move(callback)});
			return graph.IsValidVertex(handle) ? ScriptSystemId{handle, phase} : ScriptSystemId{};
		}

		[[nodiscard]] bool IsValid(const ScriptSystemId id) const noexcept {
			const Graph& graph = id.phase == core::SystemPhase::Simulation ? *Simulation : *PostCommit;
			return graph.IsValidVertex(id.handle);
		}

		[[nodiscard]] bool AddDependency(const ScriptSystemId before, const ScriptSystemId after) {
			if (before.phase != after.phase || !IsValid(before) || !IsValid(after) || before == after) return false;
			Graph& graph = Select(before.phase);
			const PoolHandle edge = graph.TryAddEdge(before.handle, after.handle, Dependency{});
			if (!graph.IsValidEdge(edge)) return false;

			std::array<PoolHandle, config::MAX_SYSTEM> ordered{};
			if (graph.TopologicalSort(std::span<PoolHandle>{ordered}, SelectScratch(before.phase)) == Status::Success) return true;
			(void)graph.TryRemoveEdge(edge);
			return false;
		}

		void Run(const core::SystemPhase phase, const float deltaTime) {
			Graph& graph = Select(phase);
			std::array<PoolHandle, config::MAX_SYSTEM> ordered{};
			if (graph.TopologicalSort(std::span<PoolHandle>{ordered}, SelectScratch(phase)) != Status::Success) {
				debug::Log("Lua system graph contains an invalid dependency");
				return;
			}
			for (std::size_t index = 0; index < graph.VertexCount(); ++index) {
				Record* record = graph.TryGetVertex(ordered[index]);
				if (record == nullptr || !record->Callback.valid()) continue;
				auto result = record->Callback(deltaTime);
				if (!result.valid()) {
					sol::error error = result;
					debug::Log("Error in registered Lua system: %s", error.what());
				}
			}
		}

		[[nodiscard]] bool On(const String<>& name, sol::protected_function callback) {
			if (name.Empty() || !callback.valid()) return false;
			for (Listener& listener : Listeners) {
				if (listener.Active) continue;
				listener.Name = name;
				listener.Callback = std::move(callback);
				listener.Active = true;
				return true;
			}
			return false;
		}

		void DispatchEvent(const Id id, const String<>& name) noexcept {
			try {
				for (Listener& listener : Listeners) {
					if (!listener.Active || listener.Name != name || !listener.Callback.valid()) continue;
					auto result = listener.Callback(id);
					if (!result.valid()) {
						sol::error error = result;
						debug::Log("Error in Lua event listener: %s", error.what());
					}
				}
			} catch (const std::exception& error) {
				debug::Log("Exception in Lua event listener: %s", error.what());
			}
		}

		void Reset() noexcept {
			Simulation.emplace();
			PostCommit.emplace();
			for (Listener& listener : Listeners) {
				listener.Active = false;
				listener.Callback = {};
				listener.Name = "";
			}
		}
	};

	static ScriptScheduler scheduler{};

	bool ScriptSystem::Initialize() {
        // Open default libraries
        lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::table, sol::lib::string, sol::lib::math);

        // Bind Debug Log override for print
        lua.set_function("print", [](sol::variadic_args va) {
            std::string logMsg = "";
            for (auto target : va) {
                std::string argStr = target.as<std::string>();
                logMsg += argStr + " ";
            }
            if (!logMsg.empty()) {
                logMsg.pop_back(); // Remove trailing space
            }
            mir::debug::Log("[Lua] %s", logMsg.c_str());
        });

        // Bind Id type
        lua.new_usertype<mir::Id>("Id",
            sol::no_constructor,
            sol::meta_function::to_string, [](const mir::Id& id) {
                return "EntityId(" + std::to_string(static_cast<std::size_t>(id)) + ")";
            },
            "index", &mir::Id::Index,
            "generation", &mir::Id::Generation
        );

        // Bind Manager singleton
        auto manager_type = lua.new_usertype<mir::core::Manager>("Manager", sol::no_constructor);
        manager_type["Instance"] = &mir::core::Manager::Instance;
        manager_type["AddEntity"] = [](mir::core::Manager& m) -> mir::Id {
            return m.AddEntity();
        };
        manager_type["DeleteEntity"] = [](mir::core::Manager& m, const mir::Id id) -> bool {
            return m.DeleteEntity(id);
        };
        manager_type["IsValidEntity"] = [](mir::core::Manager& m, const mir::Id id) -> bool {
            return m.IsValidEntity(id);
        };
		manager_type["EntityCapacity"] = []() -> std::size_t { return mir::config::MAX_ENTITY; };
		manager_type["GetEntityAt"] = [](mir::core::Manager& m, const std::size_t index) -> mir::Id {
			return m.GetActiveEntityId(index);
		};
		manager_type["ForEachEntity"] = [](mir::core::Manager& m, sol::protected_function callback) -> bool {
			if (!callback.valid()) return false;
			for (std::size_t index = 0; index < mir::config::MAX_ENTITY; ++index) {
				const mir::Id id = m.GetActiveEntityId(index);
				if (!m.IsValidEntity(id)) continue;
				auto result = callback(id);
				if (!result.valid()) return false;
			}
			return true;
		};

        // Bind Sound functions
        auto sound = lua.create_table();
        sound["Load"] = [](const std::string& name) -> bool {
            return mir::sound::Load(name.c_str());
        };
        sound["Play"] = [](const std::string& name, float volume, float pitch) {
            mir::sound::Play(name.c_str(), volume, pitch);
        };
        sound["PlayAt"] = [](const std::string& name, float x, float y, float volume, float pitch) {
            mir::sound::PlayAt(name.c_str(), x, y, volume, pitch);
        };
        sound["PlayBgm"] = [](const std::string& name, float volume, bool loop) {
            mir::sound::PlayBgm(name.c_str(), volume, loop);
        };
        sound["StopBgm"] = &mir::sound::StopBgm;
        sound["SetBgmVolume"] = &mir::sound::SetBgmVolume;
        sound["SetMasterVolume"] = &mir::sound::SetMasterVolume;
        sound["StopAll"] = &mir::sound::StopAll;
        lua["Sound"] = sound;

        // Bind Texture functions
        auto texture = lua.create_table();
        texture["Load"] = [](const std::string& name) -> bool {
            return mir::texture::Load(name.c_str());
        };
        lua["Texture"] = texture;

        // Bind Font functions
        auto font = lua.create_table();
        font["Load"] = [](const std::string& name) -> bool {
            return mir::font::Load(name.c_str());
        };
        lua["Font"] = font;

		// Bind fixed-capacity animation data. Lua receives values, never the
		// registry's internal ZET storage.
		lua.new_usertype<mir::animation::Frame>("AnimationFrame",
			sol::constructors<mir::animation::Frame(), mir::animation::Frame(float, float, float, float)>(),
			"x", &mir::animation::Frame::X,
			"y", &mir::animation::Frame::Y,
			"width", &mir::animation::Frame::Width,
			"height", &mir::animation::Frame::Height
		);
		auto animation = lua.create_table();
		animation["Register"] = [](const std::string& name, sol::table luaFrames) -> bool {
			mir::animation::Frames frames{};
			for (std::size_t index = 1; index <= luaFrames.size(); ++index) {
				sol::object value = luaFrames[index];
				mir::animation::Frame frame{};
				if (value.is<mir::animation::Frame>()) {
					frame = value.as<mir::animation::Frame>();
				} else if (value.get_type() == sol::type::table) {
					sol::table source = value.as<sol::table>();
					frame.X = source.get_or("x", source.get_or(1, 0.f));
					frame.Y = source.get_or("y", source.get_or(2, 0.f));
					frame.Width = source.get_or("width", source.get_or(3, 0.f));
					frame.Height = source.get_or("height", source.get_or(4, 0.f));
				} else {
					return false;
				}
				if (frame.Width <= 0.f || frame.Height <= 0.f || !frames.TryPush(frame)) return false;
			}
			return mir::animation::Register(name.c_str(), frames);
		};
		animation["Play"] = [](const mir::Id id, const std::string& name, const float speed, const bool loop) -> bool {
			return mir::animation::Play(id, name.c_str(), speed, loop);
		};
		animation["Stop"] = &mir::animation::Stop;
		lua["Animation"] = animation;

		auto resource = lua.create_table();
		resource["Register"] = [](const std::string& name, const std::string& path) -> bool {
			return mir::resource::Register(name.c_str(), path.c_str());
		};
		resource["GetPath"] = [](const std::string& name) -> std::string {
			return mir::resource::GetPath(name.c_str()).CStr();
		};
		resource["Unregister"] = [](const std::string& name) -> bool { return mir::resource::Unregister(name.c_str()); };
		resource["Clear"] = &mir::resource::Clear;
		resource["Count"] = &mir::resource::Count;
		lua["Resource"] = resource;

		auto scene = lua.create_table();
		scene["Register"] = [](const std::string& name, sol::protected_function callback) -> bool {
			if (!callback.valid()) return false;
			return mir::scene::Register(name.c_str(), [callback = std::move(callback)]() mutable {
				auto result = callback();
				if (!result.valid()) {
					sol::error error = result;
					mir::debug::Log("Error in Lua scene callback: %s", error.what());
				}
			});
		};
		scene["Load"] = [](const std::string& name) -> bool { return mir::scene::Load(name.c_str()); };
		scene["Unregister"] = [](const std::string& name) -> bool { return mir::scene::Unregister(name.c_str()); };
		scene["Clear"] = &mir::scene::Clear;
		scene["Current"] = []() -> std::string { return mir::scene::Current().CStr(); };
		scene["Count"] = &mir::scene::Count;
		lua["Scene"] = scene;

		auto event = lua.create_table();
		event["Emit"] = [](const mir::Id id, const std::string& name) -> bool { return mir::event::Emit(id, name.c_str()); };
		event["On"] = [](const std::string& name, sol::protected_function callback) -> bool {
			return ScriptSystem::Instance().OnEvent(name.c_str(), std::move(callback));
		};
		event["ClearLuaListeners"] = []() { ScriptSystem::Instance().ClearEventListeners(); };
		lua["Event"] = event;

		lua.new_usertype<mir::time::TimerHandle>("TimerHandle",
			sol::no_constructor,
			sol::meta_function::to_string, [](const mir::time::TimerHandle& handle) {
				return "TimerHandle(" + std::to_string(handle.Index) + ")";
			},
			"is_valid", &mir::time::IsValid
		);
		auto timer = lua.create_table();
		auto registerTimer = [](const float seconds, const bool loop, sol::protected_function callback) -> mir::time::TimerHandle {
			if (!callback.valid()) return {};
			return mir::time::Register(seconds, [callback = std::move(callback)]() mutable {
				try {
					auto result = callback();
					if (!result.valid()) {
						sol::error error = result;
						mir::debug::Log("Error in Lua timer callback: %s", error.what());
					}
				} catch (const std::exception& error) {
					mir::debug::Log("Exception in Lua timer callback: %s", error.what());
				}
			}, loop);
		};
		timer["After"] = [registerTimer](const float seconds, sol::protected_function callback) { return registerTimer(seconds, false, std::move(callback)); };
		timer["Every"] = [registerTimer](const float seconds, sol::protected_function callback) { return registerTimer(seconds, true, std::move(callback)); };
		timer["Cancel"] = &mir::time::Cancel;
		timer["IsValid"] = &mir::time::IsValid;
		timer["Clear"] = &mir::time::Clear;
		lua["Timer"] = timer;

		auto profiler = lua.create_table();
		profiler["Toggle"] = &mir::profile::ToggleProfile;
		profiler["SetEnabled"] = &mir::profile::SetEnabled;
		profiler["IsEnabled"] = &mir::profile::IsEnabled;
		profiler["GetFPS"] = &mir::profile::GetCurrentFPS;
		lua["Profiler"] = profiler;

		auto debugger = lua.create_table();
		debugger["Toggle"] = &mir::debug::ToggleDebug;
		debugger["IsColliderVisible"] = []() { return mir::debug::IsColliderVisible; };
		debugger["IsEntityCountVisible"] = []() { return mir::debug::IsEntityCountVisible; };
		lua["Debug"] = debugger;

		lua.new_usertype<mir::math::Vector2>("Vector2",
			sol::constructors<mir::math::Vector2(), mir::math::Vector2(float), mir::math::Vector2(float, float)>(),
			"x", &mir::math::Vector2::X,
			"y", &mir::math::Vector2::Y,
			"Dot", [](const mir::math::Vector2& lhs, const mir::math::Vector2& rhs) { return lhs.Dot(rhs); },
			"Cross", [](const mir::math::Vector2& lhs, const mir::math::Vector2& rhs) { return lhs.Cross(rhs); },
			"Length", [](const mir::math::Vector2& value) { return value.Length(); },
			"Normalized", [](const mir::math::Vector2& value) { return value.Norm(); },
			sol::meta_function::addition, [](const mir::math::Vector2& lhs, const mir::math::Vector2& rhs) { return lhs + rhs; },
			sol::meta_function::subtraction, [](const mir::math::Vector2& lhs, const mir::math::Vector2& rhs) { return lhs - rhs; },
			sol::meta_function::multiplication, [](const mir::math::Vector2& value, const float scale) { return value * scale; },
			sol::meta_function::division, [](const mir::math::Vector2& value, const float scale) { return scale != 0.f ? value / scale : mir::math::Vector2{}; }
		);
		lua.new_usertype<mir::math::Vector3>("Vector3",
			sol::constructors<mir::math::Vector3(), mir::math::Vector3(float), mir::math::Vector3(float, float, float, float)>(),
			"x", &mir::math::Vector3::X,
			"y", &mir::math::Vector3::Y,
			"z", &mir::math::Vector3::Z,
			"w", &mir::math::Vector3::W,
			"Dot", [](const mir::math::Vector3& lhs, const mir::math::Vector3& rhs) { return lhs.Dot(rhs); },
			"Cross", [](const mir::math::Vector3& lhs, const mir::math::Vector3& rhs) { return lhs.Cross(rhs); },
			"Length", [](const mir::math::Vector3& value) { return value.Length(); },
			"Normalized", [](const mir::math::Vector3& value) { return value.Norm(); },
			sol::meta_function::addition, [](const mir::math::Vector3& lhs, const mir::math::Vector3& rhs) { return lhs + rhs; },
			sol::meta_function::subtraction, [](const mir::math::Vector3& lhs, const mir::math::Vector3& rhs) { return lhs - rhs; },
			sol::meta_function::multiplication, [](const mir::math::Vector3& value, const float scale) { return value * scale; },
			sol::meta_function::division, [](const mir::math::Vector3& value, const float scale) { return scale != 0.f ? value / scale : mir::math::Vector3{}; }
		);
		lua.new_usertype<mir::math::Matrix3>("Matrix3",
			sol::constructors<mir::math::Matrix3(), mir::math::Matrix3(float)>(),
			"Get", [](const mir::math::Matrix3& matrix, const int column, const int row) -> float {
				return column >= 0 && column < 3 && row >= 0 && row < 3 ? matrix[column][row] : 0.f;
			},
			"Set", [](mir::math::Matrix3& matrix, const int column, const int row, const float value) -> bool {
				if (column < 0 || column >= 3 || row < 0 || row >= 3) return false;
				matrix[column][row] = value;
				return true;
			},
			"Transpose", [](const mir::math::Matrix3& matrix) { return matrix.Transpose(); }
		);
		lua.new_usertype<mir::math::Matrix4>("Matrix4",
			sol::constructors<mir::math::Matrix4(), mir::math::Matrix4(float)>(),
			"Get", [](const mir::math::Matrix4& matrix, const int column, const int row) -> float {
				return column >= 0 && column < 4 && row >= 0 && row < 4 ? matrix[column][row] : 0.f;
			},
			"Set", [](mir::math::Matrix4& matrix, const int column, const int row, const float value) -> bool {
				if (column < 0 || column >= 4 || row < 0 || row >= 4) return false;
				matrix[column][row] = value;
				return true;
			},
			"Transpose", [](const mir::math::Matrix4& matrix) { return matrix.Transpose(); },
			"Inverse", [](const mir::math::Matrix4& matrix) { return matrix.Inv(); }
		);
		lua.new_usertype<mir::math::Quaternion>("Quaternion",
			sol::constructors<mir::math::Quaternion(), mir::math::Quaternion(float, float, float, float)>(),
			"x", &mir::math::Quaternion::X,
			"y", &mir::math::Quaternion::Y,
			"z", &mir::math::Quaternion::Z,
			"w", &mir::math::Quaternion::W,
			"Dot", [](const mir::math::Quaternion& lhs, const mir::math::Quaternion& rhs) { return lhs.Dot(rhs); },
			"Length", [](const mir::math::Quaternion& value) { return value.Length(); },
			"Normalized", [](const mir::math::Quaternion& value) { return value.Norm(); },
			"Conjugate", [](const mir::math::Quaternion& value) { return value.Conjugate(); },
			"ToMatrix", [](const mir::math::Quaternion& value) { return value.ToMatrix(); },
			"Slerp", [](const mir::math::Quaternion& lhs, const mir::math::Quaternion& rhs, const float t) { return lhs.Slerp(rhs, t); }
		);
		auto math = lua.create_table();
		math["PI"] = mir::math::PI;
		math["RandomFloat"] = &mir::math::RandomFloat;
		math["RandomInt"] = &mir::math::GetRandomInt;
		math["Lerp"] = static_cast<float(*)(float, float, float)>(&mir::math::Lerp);
		math["ToRadian"] = &mir::math::ToRadian;
		math["ToDegree"] = &mir::math::ToDegree;
		math["Sin"] = &mir::math::Sin;
		math["Cos"] = &mir::math::Cos;
		math["Tan"] = &mir::math::Tan;
		math["Smoothstep"] = &mir::math::Smoothstep;
		math["CreateRandomVector2"] = [](const float min, const float max) { return mir::math::CreateRandomVector2(min, max); };
		math["CreateRandomVector3"] = [](const float min, const float max) { return mir::math::CreateRandomVector3(min, max); };
		math["CreateTranslation2D"] = [](const mir::math::Vector2& value) { return mir::math::CreateTranslation(value); };
		math["CreateTranslation3D"] = [](const mir::math::Vector3& value) { return mir::math::CreateTranslation(value); };
		math["CreateScale2D"] = [](const mir::math::Vector2& value) { return mir::math::CreateScale(value); };
		math["CreateScale3D"] = [](const mir::math::Vector3& value) { return mir::math::CreateScale(value); };
		math["CreateRotation2D"] = [](const float radians) { return mir::math::CreateRotation(radians); };
		math["CreateRotation3D"] = [](const mir::math::Vector3& axis, const float radians) { return mir::math::CreateRotation(axis, radians); };
		lua["Math"] = math;

        // Bind Transform Component functions
        auto transform = lua.create_table();
        transform["SetPosition"] = &mir::transform::SetPosition;
        transform["GetPositionX"] = [](mir::Id id) -> float {
            return mir::transform::PositionX::IsValidEntity(id) ? mir::transform::PositionX::Get(id) : 0.f;
        };
        transform["GetPositionY"] = [](mir::Id id) -> float {
            return mir::transform::PositionY::IsValidEntity(id) ? mir::transform::PositionY::Get(id) : 0.f;
        };
        transform["GetRotation"] = [](mir::Id id) -> float {
            return mir::transform::Rotation::IsValidEntity(id) ? mir::transform::Rotation::Get(id) : 0.f;
        };
        transform["GetScale"] = [](mir::Id id) -> float {
            return mir::transform::Scale::IsValidEntity(id) ? mir::transform::Scale::Get(id) : 1.f;
        };
		transform["GetWorldPositionX"] = [](mir::Id id) -> float {
			return mir::transform::WorldPositionX::IsValidEntity(id) ? mir::transform::WorldPositionX::Get(id) : 0.f;
		};
		transform["GetWorldPositionY"] = [](mir::Id id) -> float {
			return mir::transform::WorldPositionY::IsValidEntity(id) ? mir::transform::WorldPositionY::Get(id) : 0.f;
		};
		transform["GetWorldRotation"] = [](mir::Id id) -> float {
			return mir::transform::WorldRotation::IsValidEntity(id) ? mir::transform::WorldRotation::Get(id) : 0.f;
		};
		transform["GetWorldScale"] = [](mir::Id id) -> float {
			return mir::transform::WorldScale::IsValidEntity(id) ? mir::transform::WorldScale::Get(id) : 1.f;
		};
        transform["SetRotation"] = &mir::transform::SetRotation;
        transform["SetScale"] = &mir::transform::SetScale;
        transform["IsValid"] = [](mir::Id id) -> bool {
            return mir::transform::PositionX::IsValidEntity(id);
        };
        transform["Remove"] = [](mir::Id id) {
            mir::transform::PositionX::Remove(id);
            mir::transform::PositionY::Remove(id);
            mir::transform::Rotation::Remove(id);
            mir::transform::Scale::Remove(id);
        };
        lua["Transform"] = transform;

		// Hierarchy is a relationship index over ECS entities. The Lua API only
		// accepts entity Ids; ZET's internal handles never escape to scripts.
		auto hierarchy = lua.create_table();
		hierarchy["SetParent"] = &mir::hierarchy::SetParent;
		hierarchy["ClearParent"] = &mir::hierarchy::ClearParent;
		hierarchy["GetParent"] = &mir::hierarchy::ParentOf;
		hierarchy["GetFirstChild"] = &mir::hierarchy::FirstChildOf;
		hierarchy["GetNextSibling"] = &mir::hierarchy::NextSiblingOf;
		lua["Hierarchy"] = hierarchy;

        // Bind Sprite Component functions
        auto sprite = lua.create_table();
        sprite["SetTexture"] = [](mir::Id id, const std::string& path) -> bool {
            return mir::sprite::SetTexture(id, path.c_str());
        };
        sprite["GetTexture"] = [](mir::Id id) -> std::string {
            return mir::sprite::Texture::IsValidEntity(id) ? mir::sprite::Texture::Get(id).c_str() : "";
        };
        sprite["SetSourceSize"] = &mir::sprite::SetSourceSize;
        sprite["SetSourceRect"] = &mir::sprite::SetSourceRect;
        sprite["SetDestinationSize"] = &mir::sprite::SetDestinationSize;
        sprite["SetAnchor"] = &mir::sprite::SetAnchor;
        sprite["SetTint"] = &mir::sprite::SetTint;
        sprite["SetZindex"] = &mir::sprite::SetZindex;
        sprite["GetZindex"] = [](mir::Id id) -> std::uint16_t {
            return mir::sprite::Zindex::IsValidEntity(id) ? mir::sprite::Zindex::Get(id) : 0;
        };
        sprite["SetAlpha"] = &mir::sprite::SetAlpha;
        sprite["GetAlpha"] = [](mir::Id id) -> std::uint8_t {
            return mir::sprite::Alpha::IsValidEntity(id) ? mir::sprite::Alpha::Get(id) : 255;
        };
        sprite["IsValid"] = [](mir::Id id) -> bool {
            return mir::sprite::Texture::IsValidEntity(id);
        };
        sprite["Remove"] = [](mir::Id id) {
            mir::sprite::Texture::Remove(id);
            mir::sprite::SourceX::Remove(id);
            mir::sprite::SourceY::Remove(id);
            mir::sprite::SourceWidth::Remove(id);
            mir::sprite::SourceHeight::Remove(id);
            mir::sprite::DestinationWidth::Remove(id);
            mir::sprite::DestinationHeight::Remove(id);
            mir::sprite::AnchorX::Remove(id);
            mir::sprite::AnchorY::Remove(id);
            mir::sprite::Zindex::Remove(id);
            mir::sprite::Alpha::Remove(id);
            mir::sprite::TintRed::Remove(id);
            mir::sprite::TintGreen::Remove(id);
            mir::sprite::TintBlue::Remove(id);
        };
        lua["Sprite"] = sprite;

        // Bind Rigidbody Component functions
        auto rb = lua.create_table();
        rb["SetVelocity"] = &mir::rigidbody::SetVelocity;
        rb["GetVelocityX"] = [](mir::Id id) -> float {
            return mir::rigidbody::VelocityX::IsValidEntity(id) ? mir::rigidbody::VelocityX::Get(id) : 0.f;
        };
        rb["GetVelocityY"] = [](mir::Id id) -> float {
            return mir::rigidbody::VelocityY::IsValidEntity(id) ? mir::rigidbody::VelocityY::Get(id) : 0.f;
        };
        rb["SetGravity"] = &mir::rigidbody::SetGravity;
        rb["GetGravity"] = [](mir::Id id) -> float {
            return mir::rigidbody::Gravity::IsValidEntity(id) ? mir::rigidbody::Gravity::Get(id) : 0.f;
        };
        rb["SetOnGround"] = &mir::rigidbody::SetOnGround;
        rb["IsOnGround"] = [](mir::Id id) -> bool {
            return mir::rigidbody::OnGround::IsValidEntity(id) ? mir::rigidbody::OnGround::Get(id) : false;
        };
        rb["IsValid"] = [](mir::Id id) -> bool {
            return mir::rigidbody::VelocityX::IsValidEntity(id);
        };
        rb["Remove"] = [](mir::Id id) {
            mir::rigidbody::VelocityX::Remove(id);
            mir::rigidbody::VelocityY::Remove(id);
            mir::rigidbody::Gravity::Remove(id);
            mir::rigidbody::OnGround::Remove(id);
        };
        lua["Rigidbody"] = rb;

        // Bind Collider Component functions
        auto col = lua.create_table();
        col["SetBound"] = &mir::collider::SetBound;
        col["GetBoundX"] = [](mir::Id id) -> float {
            return mir::collider::BoundX::IsValidEntity(id) ? mir::collider::BoundX::Get(id) : 0.f;
        };
        col["GetBoundY"] = [](mir::Id id) -> float {
            return mir::collider::BoundY::IsValidEntity(id) ? mir::collider::BoundY::Get(id) : 0.f;
        };
        col["SetOffset"] = &mir::collider::SetOffset;
        col["GetOffsetX"] = [](mir::Id id) -> float {
            return mir::collider::OffsetX::IsValidEntity(id) ? mir::collider::OffsetX::Get(id) : 0.f;
        };
        col["GetOffsetY"] = [](mir::Id id) -> float {
            return mir::collider::OffsetY::IsValidEntity(id) ? mir::collider::OffsetY::Get(id) : 0.f;
        };
        col["SetShouldTrigger"] = &mir::collider::SetShouldTrigger;
        col["GetShouldTrigger"] = [](mir::Id id) -> bool {
            return mir::collider::ShouldTrigger::IsValidEntity(id) ? mir::collider::ShouldTrigger::Get(id) : false;
        };
        col["IsValid"] = [](mir::Id id) -> bool {
            return mir::collider::BoundX::IsValidEntity(id);
        };
        col["Remove"] = [](mir::Id id) {
            mir::collider::BoundX::Remove(id);
            mir::collider::BoundY::Remove(id);
            mir::collider::OffsetX::Remove(id);
            mir::collider::OffsetY::Remove(id);
            mir::collider::ShouldTrigger::Remove(id);
        };
        lua["Collider"] = col;

        // Bind persistent entity tags. One-frame events use mir::event::Emit
        // on the C++ side and are deliberately separate from this API.
        auto tag = lua.create_table();
        tag["Set"] = [](mir::Id id, const std::string& name) -> bool {
            return mir::tag::Set(id, name.c_str());
        };
        tag["Get"] = [](mir::Id id) -> std::string {
            return mir::tag::Tag::IsValidEntity(id) ? mir::tag::Tag::Get(id).c_str() : "";
        };
        tag["IsValid"] = [](mir::Id id) -> bool {
            return mir::tag::Tag::IsValidEntity(id);
        };
        tag["Remove"] = [](mir::Id id) {
            mir::tag::Tag::Remove(id);
        };
        lua["Tag"] = tag;

        // Bind Movement System
        auto movement = lua.create_table();
        movement["Update"] = &mir::movement::Update;
        lua["Movement"] = movement;

        // Bind Collision System
        auto collision = lua.create_table();
        collision["Update"] = &mir::collision::Update;
        lua["Collision"] = collision;

		lua.new_enum<mir::core::SystemPhase>("SystemPhase", {
			{"Simulation", mir::core::SystemPhase::Simulation},
			{"PostCommit", mir::core::SystemPhase::PostCommit}
		});
		lua.new_usertype<ScriptSystemId>("ScriptSystemId",
			sol::no_constructor,
			sol::meta_function::to_string, [](const ScriptSystemId&) { return "ScriptSystemId"; },
			sol::meta_function::equal_to, [](const ScriptSystemId& lhs, const ScriptSystemId& rhs) { return lhs == rhs; },
			"is_valid", [](const ScriptSystemId id) { return ScriptSystem::Instance().IsValidSystem(id); }
		);
		auto systems = lua.create_table();
		systems["Register"] = sol::overload(
			[](sol::protected_function callback) { return ScriptSystem::Instance().RegisterSystem(std::move(callback)); },
			[](sol::protected_function callback, const mir::core::SystemPhase phase) {
				return ScriptSystem::Instance().RegisterSystem(std::move(callback), phase);
			}
		);
		systems["AddDependency"] = [](const ScriptSystemId before, const ScriptSystemId after) {
			return ScriptSystem::Instance().AddSystemDependency(before, after);
		};
		systems["IsValid"] = [](const ScriptSystemId id) { return ScriptSystem::Instance().IsValidSystem(id); };
		lua["System"] = systems;

        // Bind Key Enum
        lua.new_enum<mir::Key>("Key", {
            {"Escape", mir::Key::Escape},
            {"Space", mir::Key::Space},
            {"Enter", mir::Key::Enter},
            {"Up", mir::Key::Up},
            {"Left", mir::Key::Left},
            {"Down", mir::Key::Down},
            {"Right", mir::Key::Right},
            {"Tab", mir::Key::Tab},
            {"Lshift", mir::Key::Lshift},
            {"Lctrl", mir::Key::Lctrl},
            {"Lalt", mir::Key::Lalt},
            {"Rshift", mir::Key::Rshift},
            {"Rctrl", mir::Key::Rctrl},
            {"Ralt", mir::Key::Ralt},
            {"Q", mir::Key::Q},
            {"W", mir::Key::W},
            {"E", mir::Key::E},
            {"R", mir::Key::R},
            {"T", mir::Key::T},
            {"Y", mir::Key::Y},
            {"U", mir::Key::U},
            {"I", mir::Key::I},
            {"O", mir::Key::O},
            {"P", mir::Key::P},
            {"A", mir::Key::A},
            {"S", mir::Key::S},
            {"D", mir::Key::D},
            {"F", mir::Key::F},
            {"G", mir::Key::G},
            {"H", mir::Key::H},
            {"J", mir::Key::J},
            {"K", mir::Key::K},
            {"L", mir::Key::L},
            {"Z", mir::Key::Z},
            {"X", mir::Key::X},
            {"C", mir::Key::C},
            {"V", mir::Key::V},
            {"B", mir::Key::B},
            {"N", mir::Key::N},
            {"M", mir::Key::M},
			{"F1", mir::Key::F1}, {"F2", mir::Key::F2}, {"F3", mir::Key::F3},
			{"F4", mir::Key::F4}, {"F5", mir::Key::F5}, {"F6", mir::Key::F6},
			{"F7", mir::Key::F7}, {"F8", mir::Key::F8}, {"F9", mir::Key::F9},
			{"F10", mir::Key::F10}, {"F11", mir::Key::F11}, {"F12", mir::Key::F12}
        });
		lua.new_enum<mir::MouseButton>("MouseButton", {
			{"Left", mir::MouseButton::Left},
			{"Middle", mir::MouseButton::Middle},
			{"Right", mir::MouseButton::Right}
		});

        // Bind Input functions
        auto input = lua.create_table();
        input["IsPressed"] = [](mir::Key key) -> bool {
            return mir::input::IsPressed(key);
        };
        input["IsJustPressed"] = [](mir::Key key) -> bool {
            return mir::input::IsJustPressed(key);
        };
        input["IsJustReleased"] = [](mir::Key key) -> bool {
            return mir::input::IsJustReleased(key);
        };
		input["IsMousePressed"] = [](const mir::MouseButton button) { return mir::input::IsMousePressed(button); };
		input["IsMouseJustPressed"] = [](const mir::MouseButton button) { return mir::input::IsMouseJustPressed(button); };
		input["IsMouseJustReleased"] = [](const mir::MouseButton button) { return mir::input::IsMouseJustReleased(button); };
        input["GetMouseX"] = &mir::input::GetMouseX;
        input["GetMouseY"] = &mir::input::GetMouseY;
        lua["Input"] = input;

		lua.new_enum<mir::window::Mode>("WindowMode", {
			{"Windowed", mir::window::Mode::Windowed},
			{"Fullscreen", mir::window::Mode::Fullscreen},
			{"Borderless", mir::window::Mode::Borderless},
			{"Desktop", mir::window::Mode::Desktop}
		});
		lua.new_enum<mir::window::Resolution>("WindowResolution", {
			{"HD", mir::window::Resolution::HD},
			{"FHD", mir::window::Resolution::FHD},
			{"QHD", mir::window::Resolution::QHD},
			{"UHD", mir::window::Resolution::UHD},
			{"Custom", mir::window::Resolution::Custom}
		});

        // Lifecycle-owned functions remain C++ main-loop responsibilities.
        auto window = lua.create_table();
        window["Close"] = &mir::window::Close;
		window["IsOpen"] = &mir::window::IsOpen;
		window["SetTitle"] = [](const std::string& title) { mir::window::SetTitle(title.c_str()); };
		window["SetFPS"] = &mir::window::SetFPS;
		window["SetSize"] = &mir::window::SetSize;
		window["SetMode"] = &mir::window::SetMode;
		window["SetResolution"] = &mir::window::SetResolution;
        lua["Window"] = window;

		auto border = lua.create_table();
		border["SetColor"] = &mir::border::SetColor;
		border["SetPosition"] = &mir::border::SetPosition;
		border["SetSize"] = &mir::border::SetSize;
		lua["Border"] = border;

		auto label = lua.create_table();
		label["SetText"] = [](const std::string& text) { mir::label::SetText(text.c_str()); };
		label["SetFont"] = [](const std::string& fontName) { mir::label::SetFont(fontName.c_str()); };
		label["SetSize"] = &mir::label::SetSize;
		label["SetColor"] = &mir::label::SetColor;
		label["SetPosition"] = &mir::label::SetPosition;
		lua["Label"] = label;

		auto button = lua.create_table();
		button["SetText"] = [](const std::string& text) { mir::button::SetText(text.c_str()); };
		button["SetFont"] = [](const std::string& fontName) { mir::button::SetFont(fontName.c_str()); };
		button["SetBackgroundColor"] = &mir::button::SetBackgroundColor;
		button["SetForegroundColor"] = &mir::button::SetForegroundColor;
		button["SetPosition"] = &mir::button::SetPosition;
		button["SetSize"] = &mir::button::SetSize;
		lua["Button"] = button;

		auto camera = lua.create_table();
		camera["SetPosition"] = &mir::camera::SetPosition;
		camera["Follow"] = &mir::camera::Follow;
		camera["ClearFollow"] = &mir::camera::ClearFollow;
		camera["SetZoom"] = &mir::camera::SetZoom;
		camera["Shake"] = &mir::camera::Shake;
		camera["GetX"] = &mir::camera::GetX;
		camera["GetY"] = &mir::camera::GetY;
		camera["GetZoom"] = &mir::camera::GetZoom;
		camera["GetTarget"] = &mir::camera::GetTarget;
		lua["Camera"] = camera;

        // Bind zet::List types
        lua.new_usertype<zet::List<int, 100>>("ListInt",
            sol::constructors<zet::List<int, 100>()>(),
            "push", [](zet::List<int, 100>& list, int val) -> bool { return list.TryPush(val); },
            "pop", &zet::List<int, 100>::Pop,
            "clear", &zet::List<int, 100>::Clear,
            "size", &zet::List<int, 100>::Size,
            "capacity", &zet::List<int, 100>::Capacity,
            "get", [](zet::List<int, 100>& list, std::size_t luaIdx) -> sol::optional<int> {
                if (luaIdx < 1 || luaIdx > list.Size()) return sol::nullopt;
                return list[luaIdx - 1];
            },
            "set", [](zet::List<int, 100>& list, std::size_t luaIdx, int val) -> bool {
                if (luaIdx < 1 || luaIdx > list.Size()) return false;
                list[luaIdx - 1] = val;
                return true;
            }
        );

        lua.new_usertype<zet::List<float, 100>>("ListFloat",
            sol::constructors<zet::List<float, 100>()>(),
            "push", [](zet::List<float, 100>& list, float val) -> bool { return list.TryPush(val); },
            "pop", &zet::List<float, 100>::Pop,
            "clear", &zet::List<float, 100>::Clear,
            "size", &zet::List<float, 100>::Size,
            "capacity", &zet::List<float, 100>::Capacity,
            "get", [](zet::List<float, 100>& list, std::size_t luaIdx) -> sol::optional<float> {
                if (luaIdx < 1 || luaIdx > list.Size()) return sol::nullopt;
                return list[luaIdx - 1];
            },
            "set", [](zet::List<float, 100>& list, std::size_t luaIdx, float val) -> bool {
                if (luaIdx < 1 || luaIdx > list.Size()) return false;
                list[luaIdx - 1] = val;
                return true;
            }
        );

        lua.new_usertype<zet::List<std::string, 100>>("ListString",
            sol::constructors<zet::List<std::string, 100>()>(),
            "push", [](zet::List<std::string, 100>& list, std::string val) -> bool { return list.TryPush(std::move(val)); },
            "pop", &zet::List<std::string, 100>::Pop,
            "clear", &zet::List<std::string, 100>::Clear,
            "size", &zet::List<std::string, 100>::Size,
            "capacity", &zet::List<std::string, 100>::Capacity,
            "get", [](zet::List<std::string, 100>& list, std::size_t luaIdx) -> sol::optional<std::string> {
                if (luaIdx < 1 || luaIdx > list.Size()) return sol::nullopt;
                return list[luaIdx - 1];
            },
            "set", [](zet::List<std::string, 100>& list, std::size_t luaIdx, std::string val) -> bool {
                if (luaIdx < 1 || luaIdx > list.Size()) return false;
                list[luaIdx - 1] = val;
                return true;
            }
        );

        // Bind GPU devices as owned userdata. Native SDL pointer addresses never
        // cross the script boundary.
		lua.new_usertype<mir::GPUDevice>("GPUDevice",
			sol::no_constructor,
			"IsValid", &mir::GPUDevice::IsValid,
			"ClaimWindow", &mir::GPUDevice::ClaimWindow,
			"ReleaseWindow", &mir::GPUDevice::ReleaseWindow,
			"GetSwapchainFormat", &mir::GPUDevice::GetSwapchainFormat,
			"Destroy", &mir::GPUDevice::Destroy
		);
        auto gpu = lua.create_table();
        gpu["CreateDevice"] = [](std::uint32_t formats, bool debugMode) -> mir::GPUDevice {
            return mir::GPUDevice::Create(formats, debugMode);
        };
        gpu["DestroyDevice"] = [](mir::GPUDevice& device) {
            device.Destroy();
        };
        gpu["ClaimWindow"] = [](const mir::GPUDevice& device) -> bool {
            return device.ClaimWindow();
        };
        gpu["ReleaseWindow"] = [](const mir::GPUDevice& device) {
            device.ReleaseWindow();
        };
        gpu["GetSwapchainFormat"] = [](const mir::GPUDevice& device) -> int {
            return device.GetSwapchainFormat();
        };
        lua["GPU"] = gpu;

        // Bind Shader Class
        lua.new_usertype<mir::Shader>("Shader",
            sol::constructors<mir::Shader()>(),
            "LoadFromFile", [](mir::Shader& self, const mir::GPUDevice& device, const std::string& filepath, const std::string& entrypoint, int stage, std::uint32_t numSamplers, std::uint32_t numUniformBuffers) -> bool {
                return device.IsValid() && self.LoadFromFile(device.Raw(), filepath.c_str(), entrypoint.c_str(), static_cast<SDL_GPUShaderStage>(stage), numSamplers, numUniformBuffers);
            },
            "Destroy", &mir::Shader::Destroy
        );

        // Bind GPUPipeline Class
        lua.new_usertype<mir::GPUPipeline>("GPUPipeline",
            sol::constructors<mir::GPUPipeline()>(),
            "Create", [](mir::GPUPipeline& self, const mir::GPUDevice& device, const mir::Shader& vs, const mir::Shader& fs, int renderTargetFormat) -> bool {
                return device.IsValid() && self.Create(device.Raw(), vs, fs, static_cast<SDL_GPUTextureFormat>(renderTargetFormat));
            },
            "Destroy", &mir::GPUPipeline::Destroy
        );

        // Bind GPU Enums & Constants
        lua.new_enum<SDL_GPUShaderStage>("GPUShaderStage", {
            {"Vertex", SDL_GPU_SHADERSTAGE_VERTEX},
            {"Fragment", SDL_GPU_SHADERSTAGE_FRAGMENT}
        });
        
        lua.set("GPU_SHADERFORMAT_SPIRV", static_cast<std::uint32_t>(SDL_GPU_SHADERFORMAT_SPIRV));
        lua.set("GPU_SHADERFORMAT_DXIL", static_cast<std::uint32_t>(SDL_GPU_SHADERFORMAT_DXIL));
        lua.set("GPU_SHADERFORMAT_MSL", static_cast<std::uint32_t>(SDL_GPU_SHADERFORMAT_MSL));

		// Lua systems run through Manager's two phase barriers. Their own bounded
		// graph only orders Lua callbacks; C++ and Lua system internals stay separate.
		auto& manager = core::Manager::Instance();
		if (!manager.IsValidSystem(manager.RegisterSystem(&ScriptSystem::DispatchSimulation, core::SystemPhase::Simulation)) ||
			!manager.IsValidSystem(manager.RegisterSystem(&ScriptSystem::DispatchPostCommit, core::SystemPhase::PostCommit))) {
			debug::Log("Failed to register Lua system dispatchers");
			return false;
		}
		event::SetScriptDispatch(&ScriptSystem::DispatchEvent);

        // Load entrypoint script
        try {
			auto result = lua.safe_script_file("script/main.lua");
            if (!result.valid()) {
                sol::error err = result;
                mir::debug::Log("Failed to run main.lua: %s", err.what());
				return false;
            }
        } catch (const std::exception& e) {
            mir::debug::Log("Exception while running main.lua: %s", e.what());
			return false;
        }

        // Fetch lifecycle functions
        sol::protected_function luaInit = lua["Init"];
        luaUpdate = lua["Update"];

		if (luaInit.valid()) {
            auto result = luaInit();
			if (!result.valid()) {
                sol::error err = result;
				mir::debug::Log("Error in Init(): %s", err.what());
				return false;
            }
		} else {
			mir::debug::Log("Warning: Init() function not found in script/main.lua");
		}
		return true;
    }

    void ScriptSystem::Update(float deltaTime) {
        if (luaUpdate.valid()) {
            auto result = luaUpdate(deltaTime);
            if (!result.valid()) {
                sol::error err = result;
                mir::debug::Log("Error in Update(): %s", err.what());
            }
        }
    }

    void ScriptSystem::Shutdown() {
        sol::protected_function luaShutdown = lua["Shutdown"];
        if (luaShutdown.valid()) {
            auto result = luaShutdown();
            if (!result.valid()) {
                sol::error err = result;
                mir::debug::Log("Error in Shutdown(): %s", err.what());
            }
        }
        // Force garbage collection and reset
		event::SetScriptDispatch(nullptr);
		scheduler.Reset();
		time::Clear();
		scene::Clear();
        lua.collect_garbage();
    }

	ScriptSystemId ScriptSystem::RegisterSystem(sol::protected_function callback, const core::SystemPhase phase) {
		return scheduler.Register(std::move(callback), phase);
	}

	bool ScriptSystem::AddSystemDependency(const ScriptSystemId before, const ScriptSystemId after) {
		return scheduler.AddDependency(before, after);
	}

	bool ScriptSystem::IsValidSystem(const ScriptSystemId id) const noexcept {
		return scheduler.IsValid(id);
	}

	bool ScriptSystem::OnEvent(const String<>& name, sol::protected_function callback) {
		return scheduler.On(name, std::move(callback));
	}

	void ScriptSystem::ClearEventListeners() noexcept {
		for (ScriptScheduler::Listener& listener : scheduler.Listeners) {
			listener.Active = false;
			listener.Callback = {};
			listener.Name = "";
		}
	}

	void ScriptSystem::DispatchSimulation(const float deltaTime) {
		scheduler.Run(core::SystemPhase::Simulation, deltaTime);
	}

	void ScriptSystem::DispatchPostCommit(const float deltaTime) {
		scheduler.Run(core::SystemPhase::PostCommit, deltaTime);
	}

	void ScriptSystem::DispatchEvent(const Id id, const String<>& name) noexcept {
		scheduler.DispatchEvent(id, name);
	}
}
