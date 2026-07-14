#include <cengine/input/Keyboard.hpp>

namespace cengine::input {

void Keyboard::pushKey(const KeyEvent event)
{
    if (m_queue.size() >= kQueueMax)
    {
        return; // fila cheia: o evento novo cai (ver kQueueMax)
    }

    m_queue.push_back(event);
}

void Keyboard::pushHeldKey(const Key key, const bool held)
{
    m_held[static_cast<size_t>(key)] = held;
}

void Keyboard::clearHeldKeys()
{
    for (bool& held : m_held)
    {
        held = false;
    }
}

KeyEvent Keyboard::readKey()
{
    if (m_queue.empty())
    {
        return {};
    }

    const KeyEvent event = m_queue.front();
    m_queue.erase(m_queue.begin());
    return event;
}

bool Keyboard::isHeld(const Key key) const
{
    return m_held[static_cast<size_t>(key)];
}

float Keyboard::heldAxis(const Key negative, const Key positive) const
{
    const bool back = isHeld(negative);
    const bool forward = isHeld(positive);

    if (back == forward)
    {
        return 0.0f; // nenhuma, ou as duas: parado
    }

    return forward ? 1.0f : -1.0f;
}

} // namespace cengine::input
