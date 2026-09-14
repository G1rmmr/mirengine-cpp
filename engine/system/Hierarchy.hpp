#pragma once

#include "../component/Transform.hpp"

#include <container/Hierarchy.hpp>
#include <container/SparseSet.hpp>

#include <array>
#include <cmath>
#include <cstddef>
#include <optional>

namespace mir::hierarchy {
	inline void Remove(Id id) noexcept;

	namespace detail {
		struct Relation {
			Id Child;
			Id Parent;
		};

		struct NodeBinding {
			std::size_t EntityGeneration = 0;
			PoolHandle Node{};
		};

		struct ParentPayload {
			Id Child;
			Id Parent;
		};

		struct ChildPayload {
			Id Child;
		};

		struct CalculatedTransform {
			float X = 0.f;
			float Y = 0.f;
			float Rotation = 0.f;
			float Scale = 1.f;
		};

		using TreeType = Hierarchy<Id, MAX_ID + 1>;

		// Relations are the authoritative ECS data. The ZET hierarchy is rebuilt
		// as a traversal index, so it never becomes a second entity model.
		inline SparseSet<Relation, MAX_ID> Relations{};
		inline SparseSet<Id, MAX_ID> Participants{};
		inline SparseSet<NodeBinding, MAX_ID> Bindings{};
		inline std::optional<TreeType> Tree{};
		inline PoolHandle WorldRoot{};
		inline TreeType::DepthFirstScratch TraversalScratch{};
		inline std::array<CalculatedTransform, MAX_ID + 1> TransformScratch{};
		inline bool TreeDirty = true;
		inline core::SystemId TransformSystem{};

		[[nodiscard]] inline Id StoredParent(const Id child) noexcept {
			if (!core::Manager::Instance().IsValidEntity(child)) return INVALID_ID;
			const Relation* relation = Relations.TryGet(child.Index);
			if (relation == nullptr || relation->Child != child ||
				!core::Manager::Instance().IsValidEntity(relation->Parent)) {
				return INVALID_ID;
			}
			return relation->Parent;
		}

		[[nodiscard]] inline bool WouldCreateCycle(const Id child, const Id parent) noexcept {
			if (child == parent) return true;
			for (Id current = parent; current != INVALID_ID; current = StoredParent(current)) {
				if (current == child) return true;
			}
			return false;
		}

		[[nodiscard]] inline PoolHandle ExistingNode(const Id id) noexcept {
			if (!Tree.has_value()) return INVALID_POOL_HANDLE;
			const NodeBinding* binding = Bindings.TryGet(id.Index);
			if (binding == nullptr || binding->EntityGeneration != id.Generation || !Tree->IsValid(binding->Node)) {
				return INVALID_POOL_HANDLE;
			}
			return binding->Node;
		}

		[[nodiscard]] inline PoolHandle BuildNode(const Id id) noexcept {
			if (!core::Manager::Instance().IsValidEntity(id) || !Tree.has_value()) return INVALID_POOL_HANDLE;
			if (const PoolHandle node = ExistingNode(id); Tree->IsValid(node)) return node;

			PoolHandle parentNode = WorldRoot;
			if (const Id parent = StoredParent(id); parent != INVALID_ID) {
				parentNode = BuildNode(parent);
				if (!Tree->IsValid(parentNode)) return INVALID_POOL_HANDLE;
			}

			const PoolHandle node = Tree->TryAddChild(parentNode, id);
			if (!Tree->IsValid(node) || Bindings.TryAssign(id.Index, NodeBinding{id.Generation, node}) == nullptr) {
				return INVALID_POOL_HANDLE;
			}
			return node;
		}

		[[nodiscard]] inline bool EnsureTree() noexcept {
			if (!TreeDirty && Tree.has_value() && Tree->IsValid(WorldRoot)) return true;

			Tree.reset();
			Bindings.Clear();
			Tree.emplace();
			WorldRoot = Tree->TryCreateRoot(INVALID_ID);
			if (!Tree->IsValid(WorldRoot)) return false;

			for (std::size_t index = 0; index < Participants.Size(); ++index) {
				const Id entity = Participants.GetAt(index);
				if (!core::Manager::Instance().IsValidEntity(entity)) continue;
				if (!Tree->IsValid(BuildNode(entity))) {
					TreeDirty = true;
					return false;
				}
			}

			TreeDirty = false;
			return true;
		}

		inline void ApplyParent(const void* rawPayload) noexcept {
			const auto& payload = *static_cast<const ParentPayload*>(rawPayload);
			auto& manager = core::Manager::Instance();
			if (!manager.IsValidEntity(payload.Child) || !manager.IsValidEntity(payload.Parent) ||
				WouldCreateCycle(payload.Child, payload.Parent)) {
				return;
			}
			if (Participants.TryAssign(payload.Child.Index, payload.Child) != nullptr &&
				Participants.TryAssign(payload.Parent.Index, payload.Parent) != nullptr &&
				Relations.TryAssign(payload.Child.Index, Relation{payload.Child, payload.Parent}) != nullptr) {
				TreeDirty = true;
			}
		}

		inline void ApplyClearParent(const void* rawPayload) noexcept {
			const auto& payload = *static_cast<const ChildPayload*>(rawPayload);
			const Relation* relation = Relations.TryGet(payload.Child.Index);
			if (relation != nullptr && relation->Child == payload.Child) {
				(void)Relations.TryRemove(payload.Child.Index);
				TreeDirty = true;
			}
		}

		inline void UpdateWorldTransforms(const float) noexcept {
			if (!EnsureTree()) return;

			(void)Tree->DepthFirst(WorldRoot, TraversalScratch, [](const PoolHandle node, const Id entity) {
				if (node == WorldRoot) {
					TransformScratch[node.Index] = CalculatedTransform{};
					return;
				}

				const PoolHandle parent = Tree->Parent(node);
				const CalculatedTransform parentTransform = Tree->IsValid(parent)
					? TransformScratch[parent.Index]
					: CalculatedTransform{};
				if (!core::Manager::Instance().IsValidEntity(entity)) return;

				const float* localX = transform::PositionX::TryGet(entity);
				const float* localY = transform::PositionY::TryGet(entity);
				const float* localRotation = transform::Rotation::TryGet(entity);
				const float* localScale = transform::Scale::TryGet(entity);
				constexpr float DegreesToRadians = 0.01745329251994329577f;
				const float radians = parentTransform.Rotation * DegreesToRadians;
				const float scaledX = (localX ? *localX : 0.f) * parentTransform.Scale;
				const float scaledY = (localY ? *localY : 0.f) * parentTransform.Scale;
				const CalculatedTransform world{
					parentTransform.X + std::cos(radians) * scaledX - std::sin(radians) * scaledY,
					parentTransform.Y + std::sin(radians) * scaledX + std::cos(radians) * scaledY,
					parentTransform.Rotation + (localRotation ? *localRotation : 0.f),
					parentTransform.Scale * (localScale ? *localScale : 1.f)
				};

				TransformScratch[node.Index] = world;
				(void)transform::SetWorldTransform(entity, world.X, world.Y, world.Rotation, world.Scale);
			});
		}

		[[nodiscard]] inline bool EnsureTransformSystem() noexcept {
			auto& manager = core::Manager::Instance();
			if (manager.IsValidSystem(TransformSystem)) return true;
			TransformSystem = manager.RegisterSystem(&UpdateWorldTransforms, core::SystemPhase::PostCommit);
			return manager.IsValidSystem(TransformSystem);
		}
	}

	[[nodiscard]] inline bool SetParent(const Id child, const Id parent) noexcept {
		auto& manager = core::Manager::Instance();
		if (!manager.IsValidEntity(child) || !manager.IsValidEntity(parent) ||
			detail::WouldCreateCycle(child, parent) || !manager.RegisterCleanup(&Remove) ||
			!transform::RegisterWorldTransformStorage() || !detail::EnsureTransformSystem()) {
			return false;
		}
		return manager.Enqueue<detail::ParentPayload>(&detail::ApplyParent, detail::ParentPayload{child, parent});
	}

	[[nodiscard]] inline bool ClearParent(const Id child) noexcept {
		auto& manager = core::Manager::Instance();
		if (!manager.IsValidEntity(child) || !manager.RegisterCleanup(&Remove) || !detail::EnsureTransformSystem()) {
			return false;
		}
		return manager.Enqueue<detail::ChildPayload>(&detail::ApplyClearParent, detail::ChildPayload{child});
	}

	[[nodiscard]] inline Id ParentOf(const Id child) noexcept {
		return detail::StoredParent(child);
	}

	[[nodiscard]] inline Id FirstChildOf(const Id parent) noexcept {
		if (!detail::EnsureTree()) return INVALID_ID;
		const PoolHandle node = detail::ExistingNode(parent);
		if (!detail::Tree->IsValid(node)) return INVALID_ID;
		const Id* value = detail::Tree->TryGet(detail::Tree->FirstChild(node));
		return value != nullptr && core::Manager::Instance().IsValidEntity(*value) ? *value : INVALID_ID;
	}

	[[nodiscard]] inline Id NextSiblingOf(const Id sibling) noexcept {
		if (!detail::EnsureTree()) return INVALID_ID;
		const PoolHandle node = detail::ExistingNode(sibling);
		if (!detail::Tree->IsValid(node)) return INVALID_ID;
		const Id* value = detail::Tree->TryGet(detail::Tree->NextSibling(node));
		return value != nullptr && core::Manager::Instance().IsValidEntity(*value) ? *value : INVALID_ID;
	}

	// Deletion intentionally orphans direct children. A future cascading-delete
	// API can reserve one batch in Manager before it mutates any entity.
	inline void Remove(const Id id) noexcept {
		(void)detail::Participants.TryRemove(id.Index);
		if (const detail::Relation* own = detail::Relations.TryGet(id.Index); own != nullptr && own->Child == id) {
			(void)detail::Relations.TryRemove(id.Index);
		}
		for (std::size_t index = 0; index < detail::Relations.Size();) {
			const detail::Relation relation = detail::Relations.GetAt(index);
			if (relation.Parent == id) {
				(void)detail::Relations.TryRemove(relation.Child.Index);
				continue;
			}
			++index;
		}
		detail::TreeDirty = true;
	}
}
