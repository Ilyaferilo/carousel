#pragma once

#include <QList>

template<typename T>
class CircleList
{
public:
    const QList<T>& list() const noexcept
    {
        return m_list;
    }

    void reset()
    {
        bool rotateLeft = m_shift > 0;
        if (rotateLeft) {
            rotateL(m_shift);
        } else {
            rotateR(-1 * m_shift);
        }
    }

    template<typename TV>
    void push(TV&& val)
    {
        if (m_shift == 0) {
            m_list.append(std::forward<TV>(val));
        } else if (m_shift > 0) {
            m_list.insert(m_shift++, std::forward<TV>(val));
        } else {
            m_list.insert(m_list.end() + m_shift, std::forward<TV>(val));
        }
    }

    void rotate(int count = 1)
    {
        if (count > 0) {
            rotateR(count);
        } else {
            rotateL(-1 * count);
        }
    }

    void rotateL(int count = 1)
    {
        if (m_list.empty()) {
            return;
        }
        m_shift = (m_shift - count) % m_list.size();
        for (size_t i = 0; i < count; i++) {
            m_list.append(m_list.takeFirst());
        }
    }

    void rotateR(int count = 1)
    {
        if (m_list.empty()) {
            return;
        }
        m_shift = (m_shift + count) % m_list.size();
        for (size_t i = 0; i < count; i++) {
            m_list.prepend(m_list.takeLast());
        }
    }

private:
    QList<T> m_list;
    int m_shift{};
};
