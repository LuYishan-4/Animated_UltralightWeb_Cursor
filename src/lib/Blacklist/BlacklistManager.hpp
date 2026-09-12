#pragma once

#include <vector>
#include <string>


namespace UltralightWebCursorM
{

class BlacklistManager
{
public:

    void setBlacklist(
        const std::vector<std::string>& list
    );


    bool contains(
        const std::string& app
    ) const;


    const std::vector<std::string>& list() const;


private:

    std::vector<std::string> blacklist_;

};

}