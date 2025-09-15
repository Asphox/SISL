#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#define SISL_IMPLEMENTATION
//#define SISL_USE_LOCK_FREE_RING_QUEUE
#include "../sisl.hpp"

#include <string>
#include <memory>
#include <iostream>

// --- HELPER CLASSES AND FUNCTIONS FOR TESTS ---

/**
 * @class Emitter
 * @brief A simple class that emits signals.
 */
class Emitter
{
public:
    sisl_signal(int_signal, int);
    sisl_signal(string_signal, const std::string&);
    sisl_signal(empty_signal);
};

/**
 * @class Receiver
 * @brief A simple class that receives signals via its slots (member functions).
 */
class Receiver
{
public:
    int m_value = 0;
    int m_counter = 0;
    std::string m_string_value;

    void receive_int(int value)
    {
        m_value = value;
        m_counter++;
    }

    void receive_string(const std::string& str)
    {
        m_string_value = str;
        m_counter++;
    }

    void reset()
    {
        m_value = 0;
        m_counter = 0;
        m_string_value = "";
    }
};

static int standalone_counter = 0;

/**
 * @brief A standalone function that can be used as a slot.
 */
void standalone_function_slot()
{
    standalone_counter++;
}

/**
 * @class SharedReceiver
 * @brief A receiver managed by a shared_ptr to test automatic disconnection.
 */
class SharedReceiver : public std::enable_shared_from_this<SharedReceiver>
{
public:
    int m_counter = 0;
    void receive() { m_counter++; }
};


// --- TEST SUITE ---

TEST_CASE("SISL Connection and Disconnection Scenarios")
{
    Emitter emitter;
    Receiver receiver;

    // ------------------------------------------------------------------
    // SCENARIO 1: Connect and disconnect a member function
    // ------------------------------------------------------------------
    SUBCASE("Connect and disconnect a specific member function")
    {
        receiver.reset();

        // Connect the `int_signal` to the `receive_int` slot
        sisl::connect(emitter, &Emitter::int_signal, receiver, &Receiver::receive_int);

        emit emitter.int_signal(42);
        CHECK(receiver.m_value == 42);
        CHECK(receiver.m_counter == 1);

        // Disconnect the specific slot
        sisl::disconnect(emitter, &Emitter::int_signal, receiver, &Receiver::receive_int);

        emit emitter.int_signal(100);
        // The value should not have changed because the slot is disconnected
        CHECK(receiver.m_value == 42);
        CHECK(receiver.m_counter == 1);
    }

    // ------------------------------------------------------------------
    // SCENARIO 2: Disconnect all slots for a specific object
    // ------------------------------------------------------------------
    SUBCASE("Disconnect all slots for a specific receiver instance")
    {
        receiver.reset();
        Receiver receiver2;

        // Connect multiple slots from the same object
        sisl::connect(emitter, &Emitter::int_signal, receiver, &Receiver::receive_int);
        sisl::connect(emitter, &Emitter::string_signal, receiver, &Receiver::receive_string);
        sisl::connect(emitter, &Emitter::int_signal, receiver2, &Receiver::receive_int); // Another object listening

        emit emitter.int_signal(50);
        emit emitter.string_signal("hello");

        CHECK(receiver.m_counter == 2);
        CHECK(receiver2.m_counter == 1);

        // Disconnect all slots associated with the `receiver` object from `int_signal`
        sisl::disconnect(emitter, &Emitter::int_signal, receiver);

        emit emitter.int_signal(99);
        emit emitter.string_signal("world");

        // The counter for `receiver` only incremented for `string_signal`
        CHECK(receiver.m_counter == 3);
        CHECK(receiver.m_value == 50); // The integer value did not change
        // `receiver2` is still connected and should have received the new value
        CHECK(receiver2.m_value == 99);
        CHECK(receiver2.m_counter == 2);
    }

    // ------------------------------------------------------------------
    // SCENARIO 3: Disconnect via the method pointer
    // ------------------------------------------------------------------
    SUBCASE("Disconnect all slots using a specific method pointer")
    {
        receiver.reset();
        Receiver receiver2;

        sisl::connect(emitter, &Emitter::int_signal, receiver, &Receiver::receive_int);
        sisl::connect(emitter, &Emitter::int_signal, receiver2, &Receiver::receive_int);

        emit emitter.int_signal(1);
        CHECK(receiver.m_counter == 1);
        CHECK(receiver2.m_counter == 1);

        // Disconnect all connections that use the `receive_int` slot
        sisl::disconnect(emitter, &Emitter::int_signal, &Receiver::receive_int);

        emit emitter.int_signal(2);
        // Neither counter should have incremented
        CHECK(receiver.m_counter == 1);
        CHECK(receiver2.m_counter == 1);
    }

    // ------------------------------------------------------------------
    // SCENARIO 4: Disconnect all slots from a signal
    // ------------------------------------------------------------------
    SUBCASE("Disconnect all slots from a signal")
    {
        receiver.reset();
        int standalone_counter = 0;

        sisl::connect(emitter, &Emitter::int_signal, receiver, &Receiver::receive_int);
        sisl::connect(emitter.int_signal, [&](int) { standalone_counter++; });

        sisl::disconnect_all(emitter, &Emitter::int_signal);

        emit emitter.int_signal(123);
        CHECK(receiver.m_counter == 0);
        CHECK(standalone_counter == 0);
    }
}

TEST_CASE("Advanced Connection Features")
{
    Emitter emitter;
    Receiver receiver;

    // ------------------------------------------------------------------
    // SCENARIO 5: Unique connection to prevent duplicates
    // ------------------------------------------------------------------
    SUBCASE("Unique connection flag prevents duplicates")
    {
        receiver.reset();

        // A normal connection can be duplicated
        sisl::connect(emitter, &Emitter::int_signal, receiver, &Receiver::receive_int);
        sisl::connect(emitter, &Emitter::int_signal, receiver, &Receiver::receive_int);
        emit emitter.int_signal(10);
        CHECK(receiver.m_counter == 2); // Slot is called twice

        sisl::disconnect_all(emitter, &Emitter::int_signal);
        receiver.reset();

        // With the `unique` flag, the second connection is ignored
        sisl::connect(emitter, &Emitter::int_signal, receiver, &Receiver::receive_int, std::thread::id(), sisl::type_connection::unique);
        sisl::connect(emitter, &Emitter::int_signal, receiver, &Receiver::receive_int, std::thread::id(), sisl::type_connection::unique);

        emit emitter.int_signal(20);
        CHECK(receiver.m_counter == 1); // Slot is called only once
    }

    // ------------------------------------------------------------------
    // SCENARIO 6: Single-shot connection
    // ------------------------------------------------------------------
    SUBCASE("Single-shot connection auto-disconnects after emission")
    {
        receiver.reset();

        sisl::connect(emitter, &Emitter::int_signal, receiver, &Receiver::receive_int, std::thread::id(), sisl::type_connection::single_shot);

        emit emitter.int_signal(30);
        CHECK(receiver.m_value == 30);
        CHECK(receiver.m_counter == 1);

        // The slot was automatically disconnected and should not be called again
        emit emitter.int_signal(40);
        CHECK(receiver.m_value == 30); // Value did not change
        CHECK(receiver.m_counter == 1);
    }
}

TEST_CASE("Other Callable Connections")
{

    SUBCASE("Lambda function connection")
    {
        sisl::signal<const std::string&> sig;
        std::string captured_value;
    
        sisl::connect(sig, [&](const std::string& val) {
            captured_value = val;
            });
    
        emit sig("lambda test");
        CHECK(captured_value == "lambda test");
        captured_value.clear();
        sisl::disconnect_all(sig);
        emit sig("after disconnect");
        CHECK(captured_value == "");
    }


    SUBCASE("Standalone function connection")
    {
        sisl::signal<> sig;
        standalone_counter = 0;
        
        sisl::connect(sig, &standalone_function_slot);
        
        emit sig();
        CHECK(standalone_counter == 1);
        
        // The API does not provide a way to disconnect a specific standalone function by pointer,
        // so only a full disconnect is easily testable.
        sisl::disconnect_all(sig);
        emit sig();
        CHECK(standalone_counter == 1); // The counter should not increment again
    }
}

TEST_CASE("Automatic Disconnection with Shared Objects")
{
    SUBCASE("Auto-disconnect for shared_ptr managed objects")
    {
        sisl::signal<> sig;
        auto shared_receiver = std::make_shared<SharedReceiver>();

        sisl::connect(sig, *shared_receiver, &SharedReceiver::receive);

        emit sig();
        CHECK(shared_receiver->m_counter == 1);

        // Keep a weak reference to verify destruction
        std::weak_ptr<SharedReceiver> weak_ref = shared_receiver;

        // Destroy the object by resetting the shared_ptr
        shared_receiver.reset();

        CHECK(weak_ref.expired()); // The object was properly destroyed

        // Emit the signal again. This call should not crash.
        // The library should gracefully ignore the call to the slot since the object no longer exists.
        REQUIRE_NOTHROW(emit sig());
    }
    SUBCASE("Auto-disconnect for shared_ptr")
    {
        sisl::signal<> sig;
        auto shared_receiver = std::make_shared<SharedReceiver>();

        sisl::connect(sig, shared_receiver, &SharedReceiver::receive);

        emit sig();
        CHECK(shared_receiver->m_counter == 1);

        // Keep a weak reference to verify destruction
        std::weak_ptr<SharedReceiver> weak_ref = shared_receiver;

        // Destroy the object by resetting the shared_ptr
        shared_receiver.reset();
        CHECK(weak_ref.expired()); // The object was properly destroyed

        // Emit the signal again. This call should not crash.
        // The library should gracefully ignore the call to the slot since the object no longer exists.
        REQUIRE_NOTHROW(emit sig());
    }
}

struct CCopyCounterNonMovable
{
    CCopyCounterNonMovable(int& copy_counter) : copy_counter(copy_counter)
    {
        copy_counter = 0;
    }

    CCopyCounterNonMovable(const CCopyCounterNonMovable& src) : copy_counter(src.copy_counter)
    {
        copy_counter++;
    }

    //CCopyCounter(CCopyCounter&& src) noexcept : copy_counter(src.copy_counter)
    //{
    //    
    //}

    int& copy_counter;
};

struct CCopyCounterMovable
{
    CCopyCounterMovable(int& copy_counter) : copy_counter(copy_counter)
    {
        copy_counter = 0;
    }

    CCopyCounterMovable(const CCopyCounterMovable& src) : copy_counter(src.copy_counter)
    {
        copy_counter++;
    }

    CCopyCounterMovable(CCopyCounterMovable&& src) noexcept : copy_counter(src.copy_counter)
    {
    }

    int& copy_counter;
};

struct CPerfectForwardingReceiver
{
    void on_slot_value_non_movable(CCopyCounterNonMovable) {}
    void on_slot_ref_non_movable(const CCopyCounterNonMovable&) {}
    void on_slot_value(CCopyCounterMovable) {}
    void on_slot_ref(const CCopyCounterMovable&) {}
};

void prefect_forwarding_receiver_value_non_movable(CCopyCounterNonMovable) {}
void prefect_forwarding_receiver_ref_non_movable(const CCopyCounterNonMovable&) {}
void prefect_forwarding_receiver_value(CCopyCounterMovable) {}
void prefect_forwarding_receiver_ref(const CCopyCounterMovable&) {}

TEST_CASE("Copy counter and perfect forwarding on direct connection")
{
    SUBCASE("Value to Value - method")
    {
        CPerfectForwardingReceiver c;
        int counter = 0;
        sisl::signal<CCopyCounterNonMovable> sig;
        sisl::connect(sig, c, &CPerfectForwardingReceiver::on_slot_value_non_movable);
        CCopyCounterNonMovable copy_counter(counter);
        sig(copy_counter);
        CHECK(counter == 1);
    }
    SUBCASE("Value to Ref - method")
    {
        CPerfectForwardingReceiver c;
        int counter = 0;
        sisl::signal<CCopyCounterNonMovable> sig;
        sisl::connect(sig, c, &CPerfectForwardingReceiver::on_slot_ref_non_movable);
        CCopyCounterNonMovable copy_counter(counter);
        sig(copy_counter);
        CHECK(counter == 0);
    }
    SUBCASE("Ref to Ref - method")
    {
        CPerfectForwardingReceiver c;
        int counter = 0;
        sisl::signal<CCopyCounterNonMovable&> sig;
        sisl::connect(sig, c, &CPerfectForwardingReceiver::on_slot_ref_non_movable);
        CCopyCounterNonMovable copy_counter(counter);
        sig(copy_counter);
        CHECK(counter == 0);
    }

    SUBCASE("Value to Value - C function")
    {
        int counter = 0;
        sisl::signal<CCopyCounterNonMovable> sig;
        sisl::connect(sig, &prefect_forwarding_receiver_value_non_movable);
        CCopyCounterNonMovable copy_counter(counter);
        sig(copy_counter);
        CHECK(counter == 1);
    }
    SUBCASE("Value to Ref - C function")
    {
        int counter = 0;
        sisl::signal<CCopyCounterNonMovable> sig;
        sisl::connect(sig, &prefect_forwarding_receiver_ref_non_movable);
        CCopyCounterNonMovable copy_counter(counter);
        sig(copy_counter);
        CHECK(counter == 0);
    }
    SUBCASE("Ref to Ref - C function")
    {
        int counter = 0;
        sisl::signal<CCopyCounterNonMovable&> sig;
        sisl::connect(sig, &prefect_forwarding_receiver_ref_non_movable);
        CCopyCounterNonMovable copy_counter(counter);
        sig(copy_counter);
        CHECK(counter == 0);
    }

    SUBCASE("Value to Value - functor")
    {
        int counter = 0;
        sisl::signal<CCopyCounterNonMovable> sig;
        sisl::connect(sig, [](CCopyCounterNonMovable c) { (void)c; });
        CCopyCounterNonMovable copy_counter(counter);
        sig(copy_counter);
        CHECK(counter == 1);
    }
    SUBCASE("Value to Ref - functor")
    {
        int counter = 0;
        sisl::signal<CCopyCounterNonMovable> sig;
        sisl::connect(sig, [](const CCopyCounterNonMovable& c) { (void)c; });
        CCopyCounterNonMovable copy_counter(counter);
        sig(copy_counter);
        CHECK(counter == 0);
    }
    SUBCASE("Ref to Ref - functor")
    {
        int counter = 0;
        sisl::signal<CCopyCounterNonMovable&> sig;
        sisl::connect(sig, [](const CCopyCounterNonMovable& c) { (void)c; });
        CCopyCounterNonMovable copy_counter(counter);
        sig(copy_counter);
        CHECK(counter == 0);
    }
}

// In queued connection SISL guarantees perfect forwarding and minimal copies only for movable types.
TEST_CASE("Copy counter and perfect forwarding on queued connection")
{
    SUBCASE("Value to Value - method")
    {
        int counter = 0;
        {
            CPerfectForwardingReceiver c;
            sisl::jthread worker([](std::stop_token token)
            {
                while (!token.stop_requested() && sisl::poll() != sisl::polling_result::terminated)
                {

                }
            });
            sisl::signal<CCopyCounterMovable> sig;
            sisl::connect(sig, c, &CPerfectForwardingReceiver::on_slot_value, worker.get_id(), sisl::type_connection::blocking_queued);
            CCopyCounterMovable copy_counter(counter);
            sig(copy_counter);
        }
        CHECK(counter == 2); // Might be optimized if arg is movable but I don't know how ...
    }
    SUBCASE("Value to Ref - method")
    {
        int counter = 0;
        CPerfectForwardingReceiver c;
        sisl::jthread worker([](std::stop_token token)
        {
            while (!token.stop_requested() && sisl::poll() != sisl::polling_result::terminated)
            {

            }
        });
        sisl::signal<CCopyCounterMovable> sig;
        sisl::connect(sig, c, &CPerfectForwardingReceiver::on_slot_ref, worker.get_id(), sisl::type_connection::blocking_queued);
        CCopyCounterMovable copy_counter(counter);
        sig(copy_counter);
        CHECK(counter == 1);
    }
    SUBCASE("Ref to Ref - method")
    {
        CPerfectForwardingReceiver c;
        sisl::jthread worker([](std::stop_token token)
        {
            while (!token.stop_requested() && sisl::poll() != sisl::polling_result::terminated)
            {

            }
        });
        int counter = 0;
        sisl::signal<CCopyCounterMovable&> sig;
        sisl::connect(sig, c, &CPerfectForwardingReceiver::on_slot_ref, worker.get_id(), sisl::type_connection::blocking_queued);
        CCopyCounterMovable copy_counter(counter);
        sig(copy_counter);
        CHECK(counter == 1);
    }

    SUBCASE("Value to Value - C function")
    {
        sisl::jthread worker([](std::stop_token token)
        {
            while (!token.stop_requested() && sisl::poll() != sisl::polling_result::terminated)
            {

            }
        });
        int counter = 0;
        sisl::signal<CCopyCounterMovable> sig;
        sisl::connect(sig, &prefect_forwarding_receiver_value, worker.get_id(), sisl::type_connection::blocking_queued);
        CCopyCounterMovable copy_counter(counter);
        sig(copy_counter);
        CHECK(counter == 2);
    }
    SUBCASE("Value to Ref - C function")
    {
        sisl::jthread worker([](std::stop_token token)
        {
            while (!token.stop_requested() && sisl::poll() != sisl::polling_result::terminated)
            {

            }
        });
        int counter = 0;
        sisl::signal<CCopyCounterMovable> sig;
        sisl::connect(sig, &prefect_forwarding_receiver_ref, worker.get_id(), sisl::type_connection::blocking_queued);
        CCopyCounterMovable copy_counter(counter);
        sig(copy_counter);
        CHECK(counter == 1);
    }
    SUBCASE("Ref to Ref - C function")
    {
        sisl::jthread worker([](std::stop_token token)
        {
            while (!token.stop_requested() && sisl::poll() != sisl::polling_result::terminated)
            {

            }
        });
        int counter = 0;
        sisl::signal<CCopyCounterMovable&> sig;
        sisl::connect(sig, &prefect_forwarding_receiver_ref, worker.get_id(), sisl::type_connection::blocking_queued);
        CCopyCounterMovable copy_counter(counter);
        sig(copy_counter);
        CHECK(counter == 1);
    }

    SUBCASE("Value to Value - functor")
    {
        sisl::jthread worker([](std::stop_token token)
        {
            while (!token.stop_requested() && sisl::poll() != sisl::polling_result::terminated)
            {

            }
        });
        int counter = 0;
        sisl::signal<CCopyCounterMovable> sig;
        sisl::connect(sig, [](CCopyCounterMovable c) { (void)c; }, worker.get_id(), sisl::type_connection::blocking_queued);
        CCopyCounterMovable copy_counter(counter);
        sig(copy_counter);
        CHECK(counter == 2);
    }
    SUBCASE("Value to Ref - functor")
    {
        sisl::jthread worker([](std::stop_token token)
        {
            while (!token.stop_requested() && sisl::poll() != sisl::polling_result::terminated)
            {

            }
        });
        int counter = 0;
        sisl::signal<CCopyCounterMovable> sig;
        sisl::connect(sig, [](const CCopyCounterMovable& c) { (void)c; }, worker.get_id(), sisl::type_connection::blocking_queued);
        CCopyCounterMovable copy_counter(counter);
        sig(copy_counter);
        CHECK(counter == 1);
    }
    SUBCASE("Ref to Ref - functor")
    {
        sisl::jthread worker([](std::stop_token token)
        {
            while (!token.stop_requested() && sisl::poll() != sisl::polling_result::terminated)
            {

            }
        });
        int counter = 0;
        sisl::signal<CCopyCounterMovable&> sig;
        sisl::connect(sig, [](const CCopyCounterMovable& c) { (void)c; }, worker.get_id(), sisl::type_connection::blocking_queued);
        CCopyCounterMovable copy_counter(counter);
        sig(copy_counter);
        CHECK(counter == 1);
    }
}