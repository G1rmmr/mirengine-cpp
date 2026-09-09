#include "asset/Resource.hpp"

void RegisterResourceFromSeparateTranslationUnit() {
    mir::resource::Register("cross-tu-resource", "assets/cross-tu-resource.png");
}
