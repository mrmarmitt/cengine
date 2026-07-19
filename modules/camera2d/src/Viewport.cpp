#include <cengine/camera2d/Viewport.hpp>

#include <cengine/collision2d/Intersects.hpp>

namespace cengine::camera2d {

Vec2 worldToView(const Viewport& viewport, const Vec2 world)
{
    return { world.x - viewport.origin.x, world.y - viewport.origin.y };
}

bool visible(const Viewport& viewport, const Aabb& worldRect)
{
    // A janela inflada pela margem nos quatro lados — a formula exata que os
    // dois consumidores carregavam copiada (mario @ 4a8f825, zelda @ 9658ae0).
    const Aabb view{ viewport.origin.x - viewport.cullMargin, viewport.origin.y - viewport.cullMargin,
                     viewport.size.x + 2.0f * viewport.cullMargin, viewport.size.y + 2.0f * viewport.cullMargin };
    return collision2d::intersects(worldRect, view);
}

} // namespace cengine::camera2d
