/*
MIT License

Copyright (c) 2020 Igor Korsukov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef KORS_MODULARITY_IOC_H
#define KORS_MODULARITY_IOC_H

#include <memory>
#include <mutex>

#include "context.h"
#include "modulesioc.h"
#include "injectable.h"

namespace kors::modularity {
ModulesIoC* _ioc(const ContextPtr& ctx = nullptr);
void removeIoC(const ContextPtr& ctx = nullptr);

struct StaticMutex
{
    static std::mutex mutex;
};

template<class I>
class Inject
{
public:

    Inject(const ContextPtr& ctx = nullptr)
        : m_ctx(ctx) {}

    Inject(const Injectable* o)
        : m_ctx(o->iocContext()) {}

    const std::shared_ptr<I>& get() const
    {
        if (!m_i) {
            const std::lock_guard<std::mutex> lock(StaticMutex::mutex);
            if (!m_i) {
                static std::string_view module = "";
                m_i = _ioc(m_ctx)->template resolve<I>(module);
            }
        }
        return m_i;
    }

    void set(std::shared_ptr<I> impl)
    {
        m_i = impl;
    }

    const std::shared_ptr<I>& operator()() const
    {
        return get();
    }

private:

    const ContextPtr m_ctx;
    mutable std::shared_ptr<I> m_i = nullptr;
};
}

#endif // KORS_MODULARITY_IOC_H
