#include "predefined.hpp"

#include "namespaces.hpp"
#include "objects.hpp"
#include "errors.hpp"

#include <iostream>
#include <chrono>
#include <cstdint>
#include <unistd.h>
#include <random>
#include <cmath>
#include <cstdlib>
#include <unistd.h>
#include <termios.h>
#include<fcntl.h> 

char getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
            return -1;
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &old);
    auto oldfl = fcntl(0, F_SETFL, O_NONBLOCK); 
    if (read(0, &buf, 1) < 0) {
        fcntl(0, F_SETFL, oldfl ^ O_NONBLOCK); 
        tcsetattr(0, TCSADRAIN, &old);
        return -1;
    }
    fcntl(0, F_SETFL, oldfl ^ O_NONBLOCK); 
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    tcsetattr(0, TCSADRAIN, &old);
    return (buf);
}

namespace Predefined {
    void InstallSingle(Names::Name name, Object *obj) {
        Namespaces::Add(Namespaces::Current(), name, obj);
        Namespaces::Track(Namespaces::Current(), obj);
    }

    void InstallFunc(std::string name, Object *(*ptr)()) {
        Object *func = Objects::Create(Objects::FUNCTION);
        Objects::MakeReferenceable(func);
        CustomTypes::FuncFromInternal(Objects::GetFunc(func), ptr);
        InstallSingle(Names::GetName(name), func);
    }

    Object *_Print() {
        for (int i = 0; i < Namespaces::StackSize(Namespaces::Current()); i++) {
            Object *arg = Namespaces::AccessStack(Namespaces::Current(), i);
            Object *str_version = Objects::CastToString(arg);
            std::cout << *Objects::GetString(str_version);
        }
        return NULL;
    }

    Object *_Println() {
        for (int i = 0; i < Namespaces::StackSize(Namespaces::Current()); i++) {
            Object *arg = Namespaces::AccessStack(Namespaces::Current(), i);
            Object *str_version = Objects::CastToString(arg);
            std::cout << *Objects::GetString(str_version);
        }
        std::cout << std::endl;
        return NULL;
    }

    uint64_t timeSinceEpochMillisec() {
        using namespace std::chrono;
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }

    Object *_GetTimeMs() {
        Object *res = Objects::Create(Objects::INT);
        *Objects::GetInt(res) = timeSinceEpochMillisec();
        Namespaces::Track(Namespaces::Current(), res);
        return res;
    }

    Object *_Sleep() {
        Object *arg = Namespaces::AccessStack(Namespaces::Current(), 0);
        if (Objects::GetType(arg) == Objects::INT) {
            usleep(*Objects::GetInt(arg) * 1000);
        }
        else if (Objects::GetType(arg) == Objects::REAL) {
            usleep(*Objects::GetReal(arg) * 1000);
        }
        else {
            RuntimeError("Expected an int or a real number");
        }
        return NULL;
    }

    Object *_Exit() {
        Object *arg = Namespaces::AccessStack(Namespaces::Current(), 0);
        if (Objects::GetType(arg) == Objects::INT) {
            exit(*Objects::GetInt(arg));
        }
        else {
            RuntimeError("Expected an int or a real number");
        }
        return NULL;
    }

    Object *_Assert() {
        Object *arg1 = Namespaces::AccessStack(Namespaces::Current(), 0);
        Object *arg2 = Namespaces::AccessStack(Namespaces::Current(), 1);


        if (Objects::GetType(arg1) != Objects::BOOL) {
           RuntimeError("Assertion failed: expected a bool value\n");
        }
        if (Objects::GetType(arg2) != Objects::STRING) {
           RuntimeError("Assertion failed: message is not string\n");
        }
        if (*Objects::GetBool(arg1) == false) {
            RuntimeError("Assertion failed. Message: "+*Objects::GetString(arg2));
        }
        return NULL;
    }

    Object *_RandInt() {
        Object *arg = Namespaces::AccessStack(Namespaces::Current(), 0);
        if (Objects::GetType(arg) != Objects::INT) {
            RuntimeError("Expected an int value");
        }

        static std::random_device rd; // obtain a random number from hardware
        static std::mt19937_64 gen(rd()); // seed the generator
        std::uniform_int_distribution<INT_T> distr(0, *Objects::GetInt(arg)); // define the range

        Object *res = Objects::Create(Objects::INT);
        *Objects::GetInt(res) = distr(gen);

        Namespaces::Track(Namespaces::Current(), res);
        return res;

    }

    Object *_Sin() {
        Object *arg = Namespaces::AccessStack(Namespaces::Current(), 0);
        REAL_T val;
        if (Objects::GetType(arg) == Objects::INT) {
            val = *Objects::GetInt(arg);
        }
        else if (Objects::GetType(arg) == Objects::REAL) {
            val = *Objects::GetReal(arg);
        }
        else {
            RuntimeError("Expected an int or a real value");
        }

        Object *res = Objects::Create(Objects::REAL);
        Namespaces::Track(Namespaces::Current(), res);

        *Objects::GetReal(res) = sin(val);

        return res;
    }

    Object *_Cos() {
        Object *arg = Namespaces::AccessStack(Namespaces::Current(), 0);
        REAL_T val;
        if (Objects::GetType(arg) == Objects::INT) {
            val = *Objects::GetInt(arg);
        }
        else if (Objects::GetType(arg) == Objects::REAL) {
            val = *Objects::GetReal(arg);
        }
        else {
            RuntimeError("Expected an int or a real value");
        }

        Object *res = Objects::Create(Objects::REAL);
        Namespaces::Track(Namespaces::Current(), res);

        *Objects::GetReal(res) = cos(val);

        return res;
    }

    Object *_Abs() {
        Object *arg = Namespaces::AccessStack(Namespaces::Current(), 0);
        if (Objects::GetType(arg) == Objects::INT) {
            INT_T val = *Objects::GetInt(arg);

            Object *res = Objects::Create(Objects::INT);
            Namespaces::Track(Namespaces::Current(), res);

            *Objects::GetInt(res) = ((val > 0) ? val : -val);

            return res;
        }
        else if (Objects::GetType(arg) == Objects::REAL) {
            REAL_T val = *Objects::GetReal(arg);

            Object *res = Objects::Create(Objects::REAL);
            Namespaces::Track(Namespaces::Current(), res);

            *Objects::GetReal(res) = ((val > 0) ? val : -val);

            return res;
        }
        else {
            RuntimeError("Expected an int or a real value");
        }
    }

    Object *_ClearTerminal() {
        system("clear");
        return NULL;
    }

    Object *_Getch() {
        Object *res = Objects::Create(Objects::CHAR);
        Namespaces::Track(Namespaces::Current(), res);
        *Objects::GetChar(res) = getch();
        return res;
    }


    void Install() {
        InstallFunc("print", _Print);
        InstallFunc("println", _Println);
        InstallFunc("gettimems", _GetTimeMs);
        InstallFunc("sleep", _Sleep);
        InstallFunc("exit", _Exit);
        InstallFunc("assert", _Assert);
        InstallFunc("randint", _RandInt);
        InstallFunc("sin", _Sin);
        InstallFunc("cos", _Cos);
        InstallFunc("abs", _Abs);
        InstallFunc("clearterminal", _ClearTerminal);
        InstallFunc("getch", _Getch);
    }
}