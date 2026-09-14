#include "asset/Resource.hpp"

void RegisterResourceFromSeparateTranslationUnit() {
	static_cast<void>(mir::resource::Register("cross-tu-resource", "assets/cross-tu-resource.png"));
}
