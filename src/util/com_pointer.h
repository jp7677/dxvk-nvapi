#pragma once

#include "../nvapi_private.h"
#include <initializer_list>

namespace dxvk {
    /**
     * \brief COM pointer
     *
     * Implements automatic reference
     * counting for COM objects.
     */
    template <typename T>
    class Com {

      public:
        Com() = default;
        Com(std::nullptr_t) {}
        Com(T* object) : m_ptr(object) {
            this->incRef();
        }

        Com(const Com& other) : m_ptr(other.m_ptr) {
            this->incRef();
        }

        Com(Com&& other) : m_ptr(other.m_ptr) {
            other.m_ptr = nullptr;
        }

        Com& operator=(T* object) {
            this->decRef();
            m_ptr = object;
            this->incRef();
            return *this;
        }

        Com& operator=(const Com& other) {
            other.incRef();
            this->decRef();
            m_ptr = other.m_ptr;
            return *this;
        }

        Com& operator=(Com&& other) {
            this->decRef();
            this->m_ptr = other.m_ptr;
            other.m_ptr = nullptr;
            return *this;
        }

        Com& operator=(std::nullptr_t) {
            this->decRef();
            m_ptr = nullptr;
            return *this;
        }

        ~Com() {
            this->decRef();
        }

        T* operator->() const {
            return m_ptr;
        }

        T** operator&() { return &m_ptr; }
        T* const* operator&() const { return &m_ptr; }

        bool operator==(const Com<T>& other) const { return m_ptr == other.m_ptr; }
        bool operator!=(const Com<T>& other) const { return m_ptr != other.m_ptr; }

        bool operator==(const T* other) const { return m_ptr == other; }
        bool operator!=(const T* other) const { return m_ptr != other; }

        bool operator==(std::nullptr_t) const { return m_ptr == nullptr; }
        bool operator!=(std::nullptr_t) const { return m_ptr != nullptr; }

        T* ref() const {
            this->incRef();
            return m_ptr;
        }

        T* ptr() const {
            return m_ptr;
        }

      private:
        T* m_ptr = nullptr;

        void incRef() const {
            if (m_ptr != nullptr)
                m_ptr->AddRef();
        }

        void decRef() const {
            if (m_ptr != nullptr)
                m_ptr->Release();
        }
    };

    template <typename T>
    T* ref(T* object) {
        if (object != nullptr)
            object->AddRef();

        return object;
    }

    // Walks a COM interface chain via QueryInterface and returns the count of
    // consecutive tiers supported. Scans from highest to lowest under the
    // single-inheritance precondition: a successful, identity-matched QI at
    // tier N implies every lower tier is present, so the typical "top tier
    // supported" case exits in three QI calls total.
    //
    // Avoids the following issues:
    //   - Unchecked static_cast from base to a derived tier is UB in C++ but
    //     safe under COM once QI has confirmed the tier exists. Cast sites
    //     NOLINT on the strength of this probe.
    //   - Tear-offs / aggregation: QI may return an object with a different
    //     identity than the base; a later cast back to the base type lands on
    //     the wrong vtable. The IUnknown round-trip rejects such tiers and
    //     continues to lower ones.
    //
    // PRECONDITION: `chain` is a single-inheritance progression
    // (IFoo -> IFoo1 -> IFoo2). Sibling or unrelated IIDs silently mis-report
    // supported tiers.
    inline uint32_t probeInterfaceChain(IUnknown* base, std::initializer_list<IID> chain) {
        if (base == nullptr)
            return 0;

        Com<IUnknown> baseIdentity;
        if (FAILED(base->QueryInterface(IID_PPV_ARGS(&baseIdentity))))
            return 0;

        const auto begin = chain.begin();
        for (uint32_t i = static_cast<uint32_t>(chain.size()); i > 0; --i) {
            Com<IUnknown> tierPtr;
            if (FAILED(base->QueryInterface(begin[i - 1], reinterpret_cast<void**>(&tierPtr))))
                continue;

            Com<IUnknown> identity;
            if (FAILED(tierPtr->QueryInterface(IID_PPV_ARGS(&identity))))
                continue;

            if (identity != baseIdentity)
                continue; // tear-off at this tier; try lower tiers

            return i;
        }
        return 0;
    }
}
