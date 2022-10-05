#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>

namespace {

    using task = std::function<void()>;

    class GUIMockup {

    public:
        void start() {
            t_ = std::thread(&GUIMockup::render, this);
        }

        void stop() {
            stop_ = true;
            t_.join();
        }

        void invokeLater(task f) {
            std::lock_guard<std::mutex> lck(m_);
            tasks_.emplace(std::move(f));
        }

        void invokeAndWait(task f) {

            std::mutex m;
            std::condition_variable cv;
            std::unique_lock<std::mutex> lck2(m);

            bool done = false;
            {
                std::lock_guard<std::mutex> lck(m_);
                tasks_.emplace([&]{
                    f();
                    done = true;
                    cv.notify_one();
                });
            }

            cv.wait(lck2, [&done]{
                return done;
            });
        }

    private:
        std::mutex m_;
        std::thread t_;
        std::queue<task> tasks_;
        std::atomic<bool> stop_ = false;

        // this function mimics a rendering loop found in GUI systems.
        // Data related to visuals must be performed on the rendering thread.
        void render() {

            while (!stop_) {

                std::queue<task> tasks;
                {
                    std::lock_guard<std::mutex> lck(m_);
                    std::swap(tasks, tasks_);
                }

                while (!tasks.empty()) {
                    auto task = tasks.front();
                    task();
                    tasks.pop();
                    if (stop_) break;
                }
            }
        }
    };

}// namespace

int main() {

    GUIMockup mockup;
    mockup.start();// starts a new thread

    bool invokeAndWaitCalled = false;
    bool invokeLaterCalled = false;

    // invoking this function should block until the lambda is processed.
    mockup.invokeAndWait([&invokeAndWaitCalled] {
        std::cout << "Task submitted through invokeAndWait executed!" << std::endl;
        invokeAndWaitCalled = true;
    });

    if (!invokeAndWaitCalled) {
        std::cerr << "Assertion failed! invokeAndWait was not called!" << std::endl;
        mockup.stop();
        return 1;
    }

    // invoking this function should not block
    mockup.invokeLater([&invokeLaterCalled] {
        std::cout << "Task submitted through invokeLater executed!" << std::endl;
        invokeLaterCalled = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (!invokeLaterCalled) {
        std::cerr << "Assertion failed! invokeLater was not called!" << std::endl;
        mockup.stop();
        return 1;
    }

    mockup.stop();

    return 0;
}