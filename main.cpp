#include <iostream>
#include <coroutine>
#include <random>
#include <ctime>

struct Game {
    struct promise_type {
        int client_guess = 0;
        int result = 0;

        Game get_return_object() {
            return Game{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }

        std::suspend_always yield_value(int value) {
            result = value;
            return {};
        }

        auto await_transform(std::suspend_always) {
            struct InputAwaiter {
                promise_type& p;
                bool await_ready() { return false; }
                void await_suspend(std::coroutine_handle<>) {}
                int await_resume() { return p.client_guess; }
            };
            return InputAwaiter{*this};
        }
    };

    using Handle = std::coroutine_handle<promise_type>;
    Handle h;

    Game(Handle h) : h(h) {}
    ~Game() { if (h) h.destroy(); }


    int make_attempt(int guess) {
        h.promise().client_guess = guess;
        h.resume();
        return h.promise().result;
    }
};


Game guess_number_game() {
    std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
    std::uniform_int_distribution<int> dist(1, 100);
    int secret = dist(rng);
    int guess = 0;
    while (true) {

        guess = co_await std::suspend_always{}; 

        int response = 0;
        if (guess < secret) response = -1;
        else if (guess > secret) response = 1;
        else response = 0;

        co_yield response;

        if (response == 0) break;
    }
}


int main() {
    setlocale(LC_ALL, "");
    
    std::cout << "Гра: Вгадай число (Варіант 8)\n";
    std::cout << "Сопрограма загадала число [1-100]. Спробуйте вгадати.\n";


    auto game = guess_number_game();

    game.h.resume(); 

    int attempt;
    int steps = 0;

    while (true) {
        std::cout << "Ваш варіант > ";
        if (!(std::cin >> attempt)) break; 
        
        steps++;

        int result = game.make_attempt(attempt);

        if (result == -1) {
            std::cout << "Сопрограма: Моє число БІЛЬШЕ (-1)\n";
        } else if (result == 1) {
            std::cout << "Сопрограма: Моє число МЕНШЕ (1)\n";
        } else {
            std::cout << "Сопрограма: ВГАДАЛИ! (0)\n";
            std::cout << "Кількість спроб: " << steps << "\n";
            break;
        }
    }

    return 0;
}
