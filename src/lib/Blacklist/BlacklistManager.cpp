#include "BlacklistManager.hpp"

#include <algorithm>
#include <cctype>


namespace UltralightWebCursorM
{


void BlacklistManager::setBlacklist(
    const std::vector<std::string>& list
)
{
    blacklist_ = list;
}



bool BlacklistManager::contains(
    const std::string& app
) const
{
    std::string lowerApp = app;


    std::transform(
        lowerApp.begin(),
        lowerApp.end(),
        lowerApp.begin(),
        [](unsigned char c)
        {
            return static_cast<char>(
                std::tolower(c)
            );
        }
    );


    for(const auto& item : blacklist_)
    {
        std::string target = item;


        std::transform(
            target.begin(),
            target.end(),
            target.begin(),
            [](unsigned char c)
            {
                return static_cast<char>(
                    std::tolower(c)
                );
            }
        );


        if(lowerApp.find(target)
            != std::string::npos)
        {
            return true;
        }
    }


    return false;
}


const std::vector<std::string>&
BlacklistManager::list() const
{
    return blacklist_;
}


}