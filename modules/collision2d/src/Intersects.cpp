#include <cengine/collision2d/Intersects.hpp>

#include <algorithm>

namespace cengine::collision2d {

bool intersects(const Aabb& a, const Aabb& b)
{
    return a.x < b.x + b.w && b.x < a.x + a.w && a.y < b.y + b.h && b.y < a.y + a.h;
}

bool intersects(const Circle& a, const Circle& b)
{
    const float dx = b.center.x - a.center.x;
    const float dy = b.center.y - a.center.y;
    const float reach = a.radius + b.radius;

    // Sem raiz quadrada: comparar os quadrados responde a mesma pergunta.
    return dx * dx + dy * dy <= reach * reach;
}

bool intersects(const Circle& circle, const Aabb& box)
{
    // O ponto do retangulo mais proximo do centro do circulo (grampeando o
    // centro nos limites do retangulo). Se ele esta ao alcance do raio, as
    // formas se tocam — e este mesmo teste cobre o circulo inteiramente dentro
    // do retangulo, caso em que o ponto mais proximo e o proprio centro.
    const float closestX = std::clamp(circle.center.x, box.x, box.x + box.w);
    const float closestY = std::clamp(circle.center.y, box.y, box.y + box.h);

    const float dx = circle.center.x - closestX;
    const float dy = circle.center.y - closestY;

    return dx * dx + dy * dy <= circle.radius * circle.radius;
}

bool intersects(const Aabb& box, const Circle& circle)
{
    return intersects(circle, box);
}

} // namespace cengine::collision2d
