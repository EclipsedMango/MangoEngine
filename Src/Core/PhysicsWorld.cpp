#include "PhysicsWorld.h"

#include <cstdarg>
#include <cstdio>
#include <thread>
#include <tracy/Tracy.hpp>

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>

#include "Nodes/AreaNode3d.h"

namespace {
    class EngineContactListener final : public JPH::ContactListener {
    public:
        struct ActiveAreaContact {
            AreaNode3d* area = nullptr;
            JPH::BodyID otherBodyId {};
            uint32_t contactCount = 0;
        };

        JPH::ValidateResult OnContactValidate(const JPH::Body&, const JPH::Body&, JPH::RVec3Arg, const JPH::CollideShapeResult&) override {
            return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
        }

        void OnContactAdded(const JPH::Body& body1, const JPH::Body& body2, const JPH::ContactManifold&, JPH::ContactSettings&) override {
            Node3d* node1 = reinterpret_cast<Node3d*>(body1.GetUserData());
            Node3d* node2 = reinterpret_cast<Node3d*>(body2.GetUserData());

            auto* area1 = dynamic_cast<AreaNode3d*>(node1);
            auto* area2 = dynamic_cast<AreaNode3d*>(node2);

            if (!area1 && !area2) {
                return;
            }

            // Ignore Area vs Area for now.
            if (area1 && area2) {
                return;
            }

            AreaNode3d* area = area1 ? area1 : area2;
            Node3d* otherNode = area1 ? node2 : node1;
            const JPH::BodyID otherBodyId = area1 ? body2.GetID() : body1.GetID();

            if (!area || !otherNode) {
                return;
            }

            const uint64_t key = MakeBodyPairKey(body1.GetID(), body2.GetID());

            const auto it = m_activeAreaContacts.find(key);
            if (it == m_activeAreaContacts.end()) {
                ActiveAreaContact contact;
                contact.area = area;
                contact.otherBodyId = otherBodyId;
                contact.contactCount = 1;

                m_activeAreaContacts[key] = contact;

                area->NotifyBodyEntered(otherBodyId, otherNode);
            } else {
                it->second.contactCount++;
            }
        }

        void OnContactPersisted(const JPH::Body&, const JPH::Body&, const JPH::ContactManifold&, JPH::ContactSettings&) override {
            // Nothing needed for simple area enter/exit.
        }

        void OnContactRemoved(const JPH::SubShapeIDPair& pair) override {
            const uint64_t key = MakeBodyPairKey(
                pair.GetBody1ID(),
                pair.GetBody2ID()
            );

            auto it = m_activeAreaContacts.find(key);

            if (it == m_activeAreaContacts.end()) {
                return;
            }

            if (it->second.contactCount > 1) {
                it->second.contactCount--;
                return;
            }

            if (it->second.area) {
                it->second.area->NotifyBodyExited(it->second.otherBodyId);
            }

            m_activeAreaContacts.erase(it);
        }

    private:
        static uint64_t MakeBodyPairKey(const JPH::BodyID a, const JPH::BodyID b) {
            const uint32_t av = a.GetIndexAndSequenceNumber();
            const uint32_t bv = b.GetIndexAndSequenceNumber();

            const uint32_t lo = std::min(av, bv);
            const uint32_t hi = std::max(av, bv);

            return (static_cast<uint64_t>(lo) << 32ull) | static_cast<uint64_t>(hi);
        }

        std::unordered_map<uint64_t, ActiveAreaContact> m_activeAreaContacts;
    };

    namespace Layers {
        constexpr JPH::ObjectLayer NON_MOVING = 0;
        constexpr JPH::ObjectLayer MOVING = 1;
        constexpr JPH::ObjectLayer NUM_LAYERS = 2;
    }

    namespace BroadPhaseLayers {
        constexpr JPH::BroadPhaseLayer NON_MOVING(0);
        constexpr JPH::BroadPhaseLayer MOVING(1);
        constexpr uint32_t NUM_LAYERS = 2;
    }

    class BroadPhaseLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
    public:
        [[nodiscard]] JPH::uint GetNumBroadPhaseLayers() const override {
            return BroadPhaseLayers::NUM_LAYERS;
        }

        [[nodiscard]] JPH::BroadPhaseLayer GetBroadPhaseLayer(const JPH::ObjectLayer inLayer) const override {
            switch (inLayer) {
                case Layers::NON_MOVING: return BroadPhaseLayers::NON_MOVING;
                case Layers::MOVING: return BroadPhaseLayers::MOVING;
                default: return BroadPhaseLayers::MOVING;
            }
        }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        [[nodiscard]] const char* GetBroadPhaseLayerName(const JPH::BroadPhaseLayer inLayer) const override {
            switch (inLayer.GetValue()) {
                case 0: return "NON_MOVING";
                case 1: return "MOVING";
                default: return "UNKNOWN";
            }
        }
#endif
    };

    class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter {
    public:
        [[nodiscard]] bool ShouldCollide(const JPH::ObjectLayer inObject1, const JPH::ObjectLayer inObject2) const override {
            if (inObject1 == Layers::NON_MOVING && inObject2 == Layers::NON_MOVING) {
                return false;
            }

            return true;
        }
    };

    class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter {
    public:
        [[nodiscard]] bool ShouldCollide(const JPH::ObjectLayer inLayer1, const JPH::BroadPhaseLayer inLayer2) const override {
            if (inLayer1 == Layers::NON_MOVING) {
                return inLayer2 == BroadPhaseLayers::MOVING;
            }

            return true;
        }
    };

    void TraceImpl(const char* inFormat, ...) {
        va_list list;
        va_start(list, inFormat);
        std::vprintf(inFormat, list);
        va_end(list);
    }

#if defined(JPH_ENABLE_ASSERTS)
    bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, JPH::uint inLine) {
        std::fprintf(stderr, "Jolt assert failed: %s (%s) in %s:%u\n", inExpression, inMessage ? inMessage : "", inFile, inLine);
        return true;
    }
#endif
}

PhysicsWorld& PhysicsWorld::Get() {
    static PhysicsWorld instance;
    return instance;
}

void PhysicsWorld::Initialize() {
    ZoneScoped;
    if (m_initialized) {
        return;
    }

    JPH::RegisterDefaultAllocator();
    JPH::Trace = TraceImpl;
#if defined(JPH_ENABLE_ASSERTS)
    JPH::AssertFailed = AssertFailedImpl;
#endif

    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();

    m_tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);

    uint32_t workerCount = std::thread::hardware_concurrency();
    if (workerCount <= 1) {
        workerCount = 1;
    } else {
        workerCount -= 1;
    }

    m_jobSystem = std::make_unique<JPH::JobSystemThreadPool>(
        JPH::cMaxPhysicsJobs,
        JPH::cMaxPhysicsBarriers,
        workerCount
    );

    m_broadPhaseLayerInterface = std::make_unique<BroadPhaseLayerInterfaceImpl>();
    m_objectVsBroadPhaseLayerFilter = std::make_unique<ObjectVsBroadPhaseLayerFilterImpl>();
    m_objectLayerPairFilter = std::make_unique<ObjectLayerPairFilterImpl>();

    constexpr JPH::uint cMaxBodies = 8192;
    constexpr JPH::uint cNumBodyMutexes = 0;
    constexpr JPH::uint cMaxBodyPairs = 8192;
    constexpr JPH::uint cMaxContactConstraints = 8192;

    m_physicsSystem.Init(
        cMaxBodies,
        cNumBodyMutexes,
        cMaxBodyPairs,
        cMaxContactConstraints,
        *m_broadPhaseLayerInterface,
        *m_objectVsBroadPhaseLayerFilter,
        *m_objectLayerPairFilter
    );

    m_contactListener = std::make_unique<EngineContactListener>();
    m_physicsSystem.SetContactListener(m_contactListener.get());

    m_initialized = true;
}

void PhysicsWorld::Shutdown() {
    if (!m_initialized) {
        return;
    }

    m_physicsSystem.SetContactListener(nullptr);
    m_contactListener.reset();

    m_objectLayerPairFilter.reset();
    m_objectVsBroadPhaseLayerFilter.reset();
    m_broadPhaseLayerInterface.reset();
    m_jobSystem.reset();
    m_tempAllocator.reset();

    JPH::UnregisterTypes();
    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;

    m_initialized = false;
}

void PhysicsWorld::Step(const float deltaTime) {
    ZoneScoped;
    if (!m_initialized || deltaTime <= 0.0f) {
        return;
    }

    constexpr int collisionSteps = 1;
    m_physicsSystem.Update(deltaTime, collisionSteps, m_tempAllocator.get(), m_jobSystem.get());
}

JPH::BodyInterface& PhysicsWorld::GetBodyInterface() {
    return m_physicsSystem.GetBodyInterface();
}
