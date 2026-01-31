#include "MessageBox.h"
#include <iostream>
#include <string>
#include "Types.h"
#include <format>
using namespace Luna;

void MessageBox(const char * title, const char * message)
{
    string safeMsg(message);
    size_t pos{};
    while ((pos = safeMsg.find("'", pos)) != string::npos) 
    {
        safeMsg.replace(pos, 1, "\\'");
        pos += 2;
    }

    string cmdTitle(title); // Convert title to string for safety with std::format
    string command = format(
        "zenity --error --title='{}' --text='{}' --width=300 2>/dev/null",
        cmdTitle, safeMsg
    );

    system(command.c_str());
}