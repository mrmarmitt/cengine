#include <cengine/anim/Animator.hpp>

namespace cengine::anim {

Animator::Animator(std::vector<ClipDesc> clips, const ClipId initial): m_clips(std::move(clips))
{
    if (initial < m_clips.size())
    {
        m_clip = initial;
    }
}

void Animator::update(const double dt, const ClipId desired)
{
    if (desired >= m_clips.size())
    {
        return; // fora do catalogo: no-op seguro (a tabela e do jogo)
    }

    if (desired != m_clip)
    {
        m_clip = desired;
        m_frame = 0;
        m_elapsed = 0.0;
        return; // troca de clip comeca no frame 0 (sem herdar o tempo do anterior)
    }

    // So ciclos multiframe avancam no tempo; poses unicas ficam no frame 0.
    // (frameTime <= 0 num ciclo multiframe seria um laco infinito — trata-se
    // como pose, em vez de girar.)
    const ClipDesc& clip = m_clips[m_clip];
    if (clip.frameCount > 1 && clip.frameTime > 0.0)
    {
        m_elapsed += dt;
        while (m_elapsed >= clip.frameTime)
        {
            m_elapsed -= clip.frameTime;
            m_frame = (m_frame + 1) % clip.frameCount;
        }
    }
    else
    {
        m_frame = 0;
        m_elapsed = 0.0;
    }
}

} // namespace cengine::anim
