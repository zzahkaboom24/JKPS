#include "../Headers/ConfigHelper.hpp"
#include "../Headers/StringHelper.hpp"
#include "../Headers/DefStrs.hpp"
#include "../Headers/Settings.hpp"
#include "../Headers/Utility.hpp"

#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <fstream>
#include <array>
#include <algorithm>
#include <cctype>
#include <cstdint>


std::string getReadingErrMsg(const LogicalParameter &par, std::string_view collection)
{
    return std::string("Failed to read '" + std::string(collection.begin(), collection.end()) 
        + " " + par.mParName + "'. Default value will be set.\n");
}

std::string getOutOfBoundsErrMsg(const LogicalParameter &par)
{
    return "Input value is out of bounds - " + par.mParName + " cannot be less than " + std::to_string(par.mLowLimits) + 
        " or more than " + std::to_string(par.mHighLimits) + ". Default value will be set.\n";
}

struct ParameterPath
{
    ParameterPath() = default;
    ParameterPath(std::string name_, std::string collection_)
        : name(name_), collection(collection_) { }

    std::string name;
    std::string collection;
};

bool operator==(const ParameterPath &lhs, const ParameterPath &rhs)
{
    return lhs.name == rhs.name && lhs.collection == rhs.collection;
}

ParameterPath getOldParName(ParameterPath parPath);

std::string cfgPath("JKPS.cfg");
std::string tmpCfgPath("tmpJKPS.cfg");
std::string errLogPath("JKPS_errlog.txt");
std::ifstream ifCfg;
std::ofstream ofCfg;
std::ifstream ifErrLog;
std::ofstream ofErrLog;

namespace ConfigHelper
{

const std::string separationSign(" $SEPARATION$ ");

bool readConfig(
    Menu::ParametersContainer &parameters, 
    const std::vector<std::string> &collectionNames)
{
    ifErrLog.open(errLogPath);
    if (ifErrLog.is_open())
    {
        ifErrLog.close();
        remove(errLogPath.c_str());
    }

    ifCfg.open(cfgPath);
    if (!ifCfg.is_open())
    {
        std::cerr << "Failed to open the config. Default config will be generated.\n";
        return false;
    }

    ofErrLog.open(errLogPath);

    readParameters(parameters, collectionNames);
    controlAssets(parameters);

    ifCfg.close();
    ofErrLog.close();

    ifErrLog.open(errLogPath);
    if (ifErrLog.is_open())
    {
        if (ifErrLog.get() == EOF)
        {
            ifErrLog.close();
            remove(errLogPath.c_str());
        }
        else
            ifErrLog.close();
    }
    return true;
}

bool isNextCollection(LogicalParameter::ID id)
{
    std::array collections {
        LogicalParameter::ID::StatTextAdvMode,
        LogicalParameter::ID::StatTextKPSText,
        LogicalParameter::ID::BtnTextFont,
        LogicalParameter::ID::BtnTextSepPosAdvMode,
        LogicalParameter::ID::BtnGfxDist,
        LogicalParameter::ID::BtnGfxAdvMode,
        LogicalParameter::ID::AnimGfxVel,
        LogicalParameter::ID::AnimGfxLight,
        LogicalParameter::ID::AnimGfxPress,
        LogicalParameter::ID::BgTxtr,
        LogicalParameter::ID::KPSWndwEn,
        LogicalParameter::ID::KeyPressVisToggle,
        LogicalParameter::ID::KeyPressVisAdvMode,
        LogicalParameter::ID::OtherSaveStats,
        LogicalParameter::ID::HotkeyAddKeys,
        LogicalParameter::ID::SaveStatMaxKPS
    };
    return std::find(collections.begin(), collections.end(), id) != collections.end();
}

void readParameters(Menu::ParametersContainer &parameters, const std::vector<std::string> &collectionNames)
{
    auto it = collectionNames.begin();
    for (auto &[id, par] : parameters)
    {
        if (isNextCollection(id))
            ++it;

        readParameter(*par, *it);
    }
}

std::queue<LogKey> getLogKeys()
{
    static const auto parName1 = std::string("Real keys");
    static const auto parName2 = std::string("Visual keys");
    auto parameterFound1 = false, parameterEmpty1 = false, null = false;

    const auto keysStr = scanParameterValue(parName1, parameterFound1, parameterEmpty1, "[" + parName1 + "]");
    const auto visualKeysStr = scanParameterValue(parName2, null, null, "[" + parName2 + "]");
         
    if (!parameterFound1 || parameterEmpty1 || keysStr == "No" || keysStr == "no" || keysStr == "NO")
        return { };

    return readKeys(keysStr, visualKeysStr);
}

std::queue<LogKey> oldGetLogKeys()
{
    static const auto parName1 = std::string("Keyboard keys");
    static const auto parName2 = std::string("Visual keys");
    static const auto oldParName1 = std::string("Keys");

    auto isParamFound = false, isParamNull = false, null = false;
    auto keysStr = scanParameterValue(parName1, isParamFound, isParamNull, "[" + parName1 + "]");
    const auto visualKeysStr = scanParameterValue(parName2, null, null, "[" + parName2 + "]");

    if (!isParamFound)
        keysStr = scanParameterValue(oldParName1, isParamFound, isParamNull, "[" + oldParName1 + "]");
         
    if (!isParamFound || isParamNull || keysStr == "No" || keysStr == "no" || keysStr == "NO")
        return { };

    return oldReadKeys(keysStr, visualKeysStr);
}

std::queue<LogKey> oldGetLogButtons()
{
    static const auto parName1 = std::string("Mouse buttons");
    static const auto parName2 = std::string("Visual buttons");
    auto isParamFound = false, isParamNull = false, null = false;

    const auto buttonsStr = scanParameterValue(parName1, isParamFound, isParamNull, "[" + parName1 + "]");
    const auto visualButtonsStr = scanParameterValue(parName2, null, null, "[" + parName2 + "]");

    if (!isParamFound || isParamNull || buttonsStr == "No" || buttonsStr == "no" || buttonsStr == "NO")
        return { };

    return oldReadButtons(buttonsStr, visualButtonsStr);
}

bool isCollection(std::string_view str)
{
    if (str.empty())
        return false;
    return str.at(0ul) == '[';
}

std::string scanParameterValue(const std::string &parName, bool &isParamFound, bool &isValueNull, std::string collection)
{
    std::ifstream cfg(cfgPath);
    assert(cfg.is_open());

    auto line = std::string();
    auto correntCollection = false;
    auto i = 0ul;

    const auto parLen = parName.length();

    while (!cfg.eof() && i != parLen)
    {
        getline(cfg, line);

        if (line.empty())
            continue;
        else if (collection == line)
            correntCollection = true;
        else if (correntCollection && isCollection(line))
            correntCollection = false;
        
        if (!correntCollection)
            continue;

        for (i = 0ul; i < parLen; ++i)
        {
            if (line[i] != parName[i])
            {
                i = 0ul;
                break;
            }
        }
        if (line[i] != ':')
            i = 0ul;
    }

    cfg.close();

    if (!correntCollection)
    {
        isParamFound = false;
        isValueNull = true;
        return "";
    }

    if (line.length() <= parName.length() + 2ul)
    {
        isParamFound = true;
        isValueNull = true;
        return "";
    }

    isParamFound = true;
    isValueNull = false;
    return std::string(line.begin() + parLen + 2ul, line.end());
}

bool isToCheck(LogicalParameter::Type type)
{
    std::array typesToCheck {
        LogicalParameter::Type::Unsigned,
        LogicalParameter::Type::Int,
        LogicalParameter::Type::Float,
        LogicalParameter::Type::Color,
        LogicalParameter::Type::VectorU,
        LogicalParameter::Type::VectorI,
        LogicalParameter::Type::VectorF
    };

    return std::find(typesToCheck.begin(), typesToCheck.end(), type) != typesToCheck.end();
}

bool isNumber(std::string_view valStr)
{
    const auto len = valStr.length();
    if (len == 0ul)
        return false;

    const auto ch = valStr.front();
    
    if (std::isdigit(ch))
    {
        return true;
    }
    else if (ch == '-' || ch == '+')
    {
        if (len <= 1ul)
            return false;
        else
            return std::isdigit(valStr.at(1ul));
    }
    return false;
}

bool canBeNull(std::string_view str)
{
    std::array strs {
        std::string("KPS text"),
        std::string("KPS text when it is 0"),
        std::string("KPS text when 0"),
        std::string("Total text"),
        std::string("BPM text")
    };

    return std::find(strs.begin(), strs.end(), str) != strs.end();
}

void readParameter(LogicalParameter &par, std::string collection)
{
    auto isParamFound = false, isValueNull = false;

    const auto canBeNullB = canBeNull(par.mParName);
    auto curParPath = ParameterPath(par.mParName, collection);
    auto valStr = scanParameterValue(curParPath.name, isParamFound, isValueNull, curParPath.collection);

    while (!isParamFound)
    {
        curParPath = getOldParName(curParPath);
        valStr = scanParameterValue(curParPath.name, isParamFound, isValueNull, curParPath.collection);

        if (isParamFound && isValueNull && canBeNullB)
            break;

        if (curParPath == ParameterPath())
        {
            std::cerr << "\"" << collection << " " << par.mParName << "\" has no alternatives on old versions\n";
            break;
        }
    }

    if (isValueNull && !canBeNullB)
    {
        if (ofErrLog.is_open())
            ofErrLog << getReadingErrMsg(par, collection);
        return;
    }

    if (valStr == "Default")
        return;

    switch(par.mType)
    {
        case LogicalParameter::Type::Unsigned: 
        case LogicalParameter::Type::Int:      
        case LogicalParameter::Type::Float:    par.setDigit(readDigitParameter(par, valStr)); break;
        case LogicalParameter::Type::Bool:     par.setBool(readBoolParameter(par, valStr));  break;
        case LogicalParameter::Type::StringPath: 
        case LogicalParameter::Type::String:   par.setString(valStr); break;
        case LogicalParameter::Type::Color:    par.setColor(readColorParameter(par, valStr)); break;
        case LogicalParameter::Type::VectorU:
        case LogicalParameter::Type::VectorI:
        case LogicalParameter::Type::VectorF:  par.setVector(readVectorParameter(par, valStr)); break;
        case LogicalParameter::Type::Hotkey:   par.setValStr(valStr); break;
        default: break;
    }

    if (par.mType == LogicalParameter::Type::StringPath)
    {
        if (par.mType == LogicalParameter::Type::StringPath 
        &&  Settings::BackgroundTexturePath == "GreenscreenBG.png")
            return;
            
        std::ifstream check(par.getString());
        if (check.is_open())
            check.close();
        else
        {
            par.setString(par.getDefValStr());
            if (ofErrLog.is_open())
                ofErrLog << getReadingErrMsg(par, collection);
        }
    }
}

float readDigitParameter(const LogicalParameter &par, std::string &valStr)
{
    auto handleError = [&] (bool useDefault) 
        { 
			const std::string errStr = getOutOfBoundsErrMsg(par);
			if (ofErrLog.is_open())
				ofErrLog << errStr;
			std::cout << errStr;
			valStr = useDefault ? par.getDefValStr() : std::to_string(static_cast<int>(par.mLowLimits));
            return readDigitParameter(par, valStr); 
        };

    const auto lowLim = par.mLowLimits, highLim = par.mHighLimits;

    if (!isNumber(valStr))
        handleError(true);

    const auto val = std::stof(valStr);

    if (lowLim > val || val > highLim)
        return handleError(false);

    return val;
}

sf::Vector2f readVectorParameter(const LogicalParameter &par, const std::string &valStr)
{
    auto handleError = [&] () 
        { 
            ofErrLog << getOutOfBoundsErrMsg(par);
            return readVectorParameter(par, par.getDefValStr()); 
        };

    const auto size = 2ul;
    float vec[size];
    const auto lowLim = par.mLowLimits, highLim = par.mHighLimits;
    
    for (auto i = 0ul, strIdx = 0ul; i < size; ++i)
    {
        const auto &substr = valStr.substr(strIdx, 81ul);
        if (!isNumber(substr))
            return handleError();

        const auto val = std::stof(substr);
        vec[i] = val;

        if (i < size - 1ul)
        {
            const auto found = substr.find(',');
            if (found == std::string::npos)
                return handleError();

            strIdx += found + 1ul;
        }

        if (val < lowLim || val > highLim)
            return handleError();
    }

    return sf::Vector2f(vec[0], vec[1]);
}

sf::Color readColorParameter(const LogicalParameter &par, const std::string &valStr)
{
    auto handleError = [&] () 
        { 
            ofErrLog << getOutOfBoundsErrMsg(par);
            return readColorParameter(par, par.getDefValStr()); 
        };

    const auto size = 4ul;
    std::uint8_t rgba[size]; // SFML 3: standard int
    const auto lowLim = par.mLowLimits, highLim = par.mHighLimits;

    for (auto i = 0ul, strIdx = 0ul; i < size; ++i)
    {
        const auto &substr = valStr.substr(strIdx, 81ul);

        if (i <= size - 1ul)
        {
            const auto found = substr.find(',');
            if (found == std::string::npos && i == size - 1ul)
            {
                if (substr.empty())
                {
                    rgba[i] = static_cast<std::uint8_t>(255);
                    continue;
                }

                if (!isNumber(substr))
                    return handleError();

                const auto val = std::stof(substr);
                rgba[i] = static_cast<std::uint8_t>(val);

                continue;
            }
            if (found == std::string::npos && ofErrLog.is_open())
                return handleError();
                
            strIdx += found + 1ul;
        }
        
        if (!isNumber(substr))
            return handleError();

        const auto val = std::stof(substr);
        rgba[i] = static_cast<std::uint8_t>(val);

        if ((val < lowLim || val > highLim) && ofErrLog.is_open())
            return handleError();
    }

    return sf::Color(rgba[0], rgba[1], rgba[2], rgba[3]);
}

bool readBoolParameter(const LogicalParameter &par, const std::string &valStr)
{
    if (valStr == "true" || valStr == "True" || valStr == "TRUE")
        return true;
    else if (valStr == "false" || valStr == "False" || valStr == "FALSE")
        return false;
    else if (ofErrLog.is_open())
        ofErrLog << getReadingErrMsg(par, "[Bool]");
    else
        return par.getDefValStr().size() == 4ul;
    
    return false;
}

template <typename T>
void controlResource(Menu::ParametersContainer &parameters, std::string filepath, 
    LogicalParameter::ID id, std::string collection)
{
    if (filepath != "Default")
    {
        T resource;
        bool loaded = false;
        
        // SFML 3: Fonts stream, so they use openFromFile. Textures use loadFromFile.
        if constexpr (std::is_same_v<T, sf::Font>)
            loaded = resource.openFromFile(filepath);
        else
            loaded = resource.loadFromFile(filepath);

        if (!loaded)
        {
            auto found = parameters.find(id);
            assert(found != parameters.end());
            auto &parm = *found->second;
            if (ofErrLog.is_open())
                ofErrLog << getReadingErrMsg(parm, collection);
            parm.resetToDefaultValue();
        }
    }
}

void controlAssets(Menu::ParametersContainer &parameters)
{
    controlResource<sf::Texture>(parameters, Settings::GfxButtonTexturePath, LogicalParameter::ID::BtnGfxTxtr, "[Button graphics]");
    controlResource<sf::Texture>(parameters, Settings::AnimationTexturePath, LogicalParameter::ID::AnimGfxTxtr, "[Animation graphics]");
    if (Settings::BackgroundTexturePath != "GreenscreenBG.png")
        controlResource<sf::Texture>(parameters, Settings::BackgroundTexturePath, LogicalParameter::ID::BgTxtr, "[Main window]");

    controlResource<sf::Font>(parameters, Settings::StatisticsTextFontPath, LogicalParameter::ID::StatTextFont, "[Statistics text]");
    controlResource<sf::Font>(parameters, Settings::ButtonTextFontPath, LogicalParameter::ID::BtnTextFont, "[Button text]");
    controlResource<sf::Font>(parameters, Settings::KPSWindowTextFontPath, LogicalParameter::ID::KPSWndwTxtFont, "[KPS window]");
    controlResource<sf::Font>(parameters, Settings::KPSWindowNumberFontPath, LogicalParameter::ID::KPSWndwNumFont, "[KPS window]");
}

std::queue<LogKey> readKeys(const std::string &keysStr, const std::string &visualKeysStr)
{
    std::queue<LogKey> logKeysQueue;
    auto strIdx1 = 0lu, strIdx2 = 0lu;
    const auto len = ConfigHelper::separationSign.length();

    for (unsigned i = 0; strIdx1 < keysStr.size(); ++i)
    {
        std::string keyStr(keysStr, strIdx1, keysStr.substr(strIdx1).find(ConfigHelper::separationSign));
        std::string visualKeyStr(visualKeysStr, strIdx2, visualKeysStr.substr(strIdx2).find(ConfigHelper::separationSign));   
        std::string checkStr;
        sf::Keyboard::Scancode scancode = sf::Keyboard::Scancode::Unknown;
        sf::Mouse::Button button;

        const bool isKeyB = isKey(keyStr);
        const bool isButtonB = isButton(keyStr);
        if (!isKeyB && !isButtonB)
        {
            if (keysStr.find(ConfigHelper::separationSign, strIdx1) == std::string::npos)
                break;
            strIdx1 = keysStr.find(ConfigHelper::separationSign, strIdx1) + len;
            strIdx2 = visualKeysStr.find(ConfigHelper::separationSign, strIdx2) + len;
        } 
        else if (isKeyB)
        {
            scancode = strToScancode(keyStr);
            checkStr = scancodeToStr(scancode, true);
        }
        else
        {
            button = strToBtn(keyStr);
            checkStr = btnToStr(button);
        }
            
        unsigned maxLength = 20u;
        if (visualKeyStr.size() > maxLength || visualKeyStr.size() == 0)
            visualKeyStr = checkStr;

        if (isKeyB)
            logKeysQueue.emplace(*new LogKey(keyStr, visualKeyStr, new sf::Keyboard::Scancode(scancode), nullptr));
        else
            logKeysQueue.emplace(*new LogKey(keyStr, visualKeyStr, nullptr, new sf::Mouse::Button(button)));

        logKeysQueue.back().realStr = checkStr == keyStr ? keyStr : checkStr;

        if (keysStr.find(ConfigHelper::separationSign, strIdx1) == std::string::npos)
            break;

        strIdx1 = keysStr.find(ConfigHelper::separationSign, strIdx1) + len;
        strIdx2 = visualKeysStr.find(ConfigHelper::separationSign, strIdx2) + len;
    }

    return logKeysQueue;
}

std::queue<LogKey> oldReadKeys(const std::string &keysStr, const std::string &visualKeysStr)
{
    std::queue<LogKey> logKeysQueue;
    unsigned strIdx1 = 0u, strIdx2 = 0u;

    for (unsigned i = 0u; strIdx1 < keysStr.size(); ++i)
    {
        std::string keyStr(keysStr, strIdx1, keysStr.substr(strIdx1).find(','));
        std::string visualKeyStr(visualKeysStr, strIdx2, visualKeysStr.substr(strIdx2).find(','));
        std::string checkStr;

        sf::Keyboard::Scancode scancode = strToScancode(keyStr);
        if (scancode == sf::Keyboard::Scancode::Unknown)
        {
            if (keysStr.find(',', strIdx1) == std::string::npos)
                break;
            strIdx1 = keysStr.find(',', strIdx1) + 1;
            strIdx2 = visualKeysStr.find(',', strIdx2) + 1;
        }

        checkStr = scancodeToStr(scancode, true);
        unsigned maxLength = 20;
        if (visualKeyStr.size() > maxLength || visualKeyStr.size() == 0)
            visualKeyStr = checkStr;

        logKeysQueue.emplace(*new LogKey(keyStr, visualKeyStr, new sf::Keyboard::Scancode(scancode), nullptr));
        logKeysQueue.back().realStr = checkStr == keyStr ? keyStr : checkStr;

        if (keysStr.find(',', strIdx1) == std::string::npos)
            break;

        strIdx1 = keysStr.find(',', strIdx1) + 1;
        strIdx2 = visualKeysStr.find(',', strIdx2) + 1;
    }

    return logKeysQueue;
}

std::queue<LogKey> oldReadButtons(const std::string &buttonsStr, const std::string &visualButtonsStr)
{
    std::queue<LogKey> logBtnQueue;
    unsigned strIdx1 = 0, strIdx2 = 0;

    for (unsigned i = 0; strIdx1 < buttonsStr.size(); ++i)
    {
        std::string buttonStr(buttonsStr, strIdx1, buttonsStr.substr(strIdx1).find(','));
        std::string visualButtonStr(visualButtonsStr, strIdx2, visualButtonsStr.substr(strIdx2).find(','));
        std::string checkStr;

        sf::Mouse::Button button = strToBtn(buttonStr);

        checkStr = btnToStr(button);
        unsigned maxLength = 20;
        if (visualButtonStr.size() > maxLength || visualButtonStr.size() == 0)
            visualButtonStr = checkStr;

        logBtnQueue.emplace(*new LogKey(buttonStr, visualButtonStr, nullptr, new sf::Mouse::Button(button)));
        logBtnQueue.back().realStr = checkStr == buttonStr ? buttonStr : checkStr;

        if (buttonsStr.find(',', strIdx1) == std::string::npos)
            break;

        strIdx1 = buttonsStr.find(',', strIdx1) + 1;
        strIdx2 = visualButtonsStr.find(',', strIdx2) + 1;
    }

    return logBtnQueue;
}

void saveConfig(
    const Menu::ParametersContainer &parameters, const Menu::ParameterLinesContainer &parameterLines,
    const std::vector<std::unique_ptr<Button>> *mKeys, bool saveKeys)
{
    ofCfg.open(tmpCfgPath);

    ofCfg << defCfgComment;
    ofCfg << "\n";

    if (saveKeys)
    {    
        ofCfg << "[Real keys]\nReal keys: " << getKeysStr(*mKeys, true);
        ofCfg << "[Visual keys]\nVisual keys: " << getKeysStr(*mKeys, false);
    }
    else
    {
        ofCfg << "[Real keys]\nReal keys: Z,X\n";
        ofCfg << "[Visual keys]\nVisual keys: Z,X\n";
    }

    bool commentsSection = false;
    auto parmPair = parameters.begin();
    auto parmLinePair = parameterLines.begin();
    while (parmPair != parameters.end() || parmLinePair != parameterLines.end())
    {
        using Ptr = std::shared_ptr<LogicalParameter>;
        Ptr parP = parmPair != parameters.end() ? parmPair->second : nullptr;
        Ptr parP2 = parmLinePair != parameterLines.end() ? parmLinePair->second->getParameter() : nullptr;
        Ptr mainParP = nullptr;

        if (parP == parP2)
        {
            mainParP = parP;
            ++parmPair; ++parmLinePair;
        }
        
        if (parmLinePair != parameterLines.end()
        && (parP2->mType == LogicalParameter::Type::Empty 
        ||  parP2->mType == LogicalParameter::Type::Collection
        ||  parP2->mType == LogicalParameter::Type::Hint))
        {
            mainParP = parP2;
            ++parmLinePair;
        } 
        else if (parmPair != parameters.end() && parP != parP2) 
        {
            mainParP = parP;
            ++parmPair;
        }

        if (commentsSection)
            ofCfg << "# ";
        if (!mainParP) {
            std::cerr << "CRITICAL: mainParP is nullptr! parP: " 
                      << (parP ? parP->mParName : "NULL") 
                      << ", parP2: " 
                      << (parP2 ? parP2->mParName : "NULL") << std::endl;
            exit(1);
        }
        std::cout << "Processing: " << mainParP->mParName << " (Type: " << static_cast<int>(mainParP->mType) << ")" << std::endl;
        ofCfg << mainParP->mParName;
        if (mainParP->mType != LogicalParameter::Type::Empty 
        &&  mainParP->mType != LogicalParameter::Type::Collection
        &&  mainParP->mType != LogicalParameter::Type::Hint)
            ofCfg << ": " << mainParP->getValStr();
        ofCfg << "\n";

        if (parmLinePair != parameterLines.end() && parmLinePair->first == ParameterLine::ID::Info1)
            commentsSection = true;
    }

    ofCfg.close();

    std::ofstream ofCfgCheck(cfgPath, std::fstream::in);
    if (ofCfgCheck.is_open())
    {
        ofCfgCheck.close();
        remove(cfgPath.c_str());
    }

    rename(tmpCfgPath.c_str(), cfgPath.c_str());
}

std::string getKeysStr(const std::vector<std::unique_ptr<Button>> &mKeys, bool readRealStr)
{
    std::string str;
    auto it = mKeys.begin();

    for (; it != mKeys.end(); ++it)
    {
        const LogKey *lKey = (*it)->getLogKey();
        assert(lKey && (lKey->keyboardKey || lKey->mouseButton));

        str += (readRealStr ? lKey->realStr : lKey->visualStr) + ConfigHelper::separationSign;
    }
    if (it != mKeys.begin())
        str.erase(str.size() - ConfigHelper::separationSign.length());
    else
        str += "No";
    str += "\n\n";

    return str;
}

} // namespace ConfigHelper

ParameterPath getOldParName(ParameterPath parPath)
{
    auto newPath = parPath;

    if (parPath.collection == "[Statistics text]")
    {
        if (parPath.name == "Distance between lines")
            newPath.name =  "Distance between";
        else if (parPath.name == "Distance between")
            newPath.name =       "Statistics text distance";
        else if (parPath.name == "Position offset")
            newPath.name =       "Statistics text position";

        else if (parPath.name == "Text position offset")
            newPath.name =       "Position offset";

        else if (parPath.name == "Value position offset")
            newPath.name =       "Number position offset";
        else if (parPath.name == "Number position offset")
            newPath.name =       "Statistics value text position";

        else if (parPath.name == "Font filepath")
            newPath.name =       "Statistics text font";

        else if (parPath.name == "Text color")
            newPath.name =       "Statistics text color";

        else if (parPath.name == "Character size")
            newPath.name =       "Statistics text character size";

        else if (parPath.name == "Outline thickness")
            newPath.name =       "Statistics text outline thickness";

        else if (parPath.name == "Outline color")
            newPath.name =       "Statistics text outline color";

        else if (parPath.name == "Bold")
            newPath.name =       "Statistics text bold";

        else if (parPath.name == "Italic")
            newPath.name =       "Statistics text italic";

        else if (parPath.name == "Enabled")
            newPath.name =       "Show statistics text";
    }

    else if (parPath.collection ==  "[Statistics text advanced settings]")
    {
        if (parPath.name == "KPS text position offset")
            newPath.name =  "Statistics KPS text position";

        else if (parPath.name == "KPS value position offset")
            newPath.name =       "KPS number text position offset";
        else if (parPath.name == "KPS number text position offset")
            newPath.name =       "Statistics KPS value text position";
            
        else if (parPath.name == "KPS text color")
            newPath.name =       "Statistics KPS text color";

        else if (parPath.name == "KPS text character size")
            newPath.name =       "Statistics KPS text character size";

        else if (parPath.name == "KPS bold")
            newPath.name =       "Statistics KPS bold";

        else if (parPath.name == "KPS italic")
            newPath.name =       "Statistics KPS italic";

        else if (parPath.name == "Total text position offset")
            newPath.name =       "Statistics total text position";

        else if (parPath.name == "Total value position offset")
            newPath.name =       "Total number text position offset";
        else if (parPath.name == "Total number text position offset")
            newPath.name =       "Statistics total value text position";

        else if (parPath.name == "Total text color")
            newPath.name =       "Statistics total text color";

        else if (parPath.name == "Total text character size")
            newPath.name =       "Statistics total text character size";

        else if (parPath.name == "Total bold")
            newPath.name =       "Statistics total bold";

        else if (parPath.name == "Total italic")
            newPath.name =       "Statistics total italic";

        else if (parPath.name == "BPM text position offset")
            newPath.name =       "Statistics BPM text position";

        else if (parPath.name == "BPM value position offset")
            newPath.name =       "BPM number text position offset";
        else if (parPath.name == "BPM number text position offset")
            newPath.name =       "Statistics BPM value text position";

        else if (parPath.name == "BPM text color")
            newPath.name =       "Statistics BPM text color";

        else if (parPath.name == "BPM text character size")
            newPath.name =       "Statistics BPM text character size";

        else if (parPath.name == "BPM bold")
            newPath.name =       "Statistics BPM bold";

        else if (parPath.name == "BPM italic")
            newPath.name =       "Statistics BPM italic";
    }

    else if (parPath.collection == "[Statistics text strings]")
    {
        if (parPath.name == "KPS text" 
        ||  parPath.name == "Total text" 
        ||  parPath.name == "BPM text")
        {
            newPath.collection = "[Statistics text advanced settings]";
        }

        else if (parPath.name == "KPS text when 0")
        {
            newPath.name = "KPS text when it is 0";
            newPath.collection = "[Statistics text advanced settings]";
        }        
    }

    else if (parPath.collection == "[Buttons text]")
    {
             if (parPath.name == "Font filepath")
            newPath.name =       "Buttons text font";

        else if (parPath.name == "Text color")
            newPath.name =       "Buttons text color";

        else if (parPath.name == "Character size")
            newPath.name =       "Buttons text character size";

        else if (parPath.name == "Outline thickness")
            newPath.name =       "Buttons text outline thickness";

        else if (parPath.name == "Outline color")
            newPath.name =       "Buttons text outline color";

        else if (parPath.name == "Position offset")
            newPath.name =       "Buttons text position";

        else if (parPath.name == "Text bounds")
            newPath.name =       "Buttons text bounds";

        else if (parPath.name == "Bold")
            newPath.name =       "Buttons text bold";

        else if (parPath.name == "Italic")
            newPath.name =       "Buttons text italic";

        else if (parPath.name == "Show key labels")
            newPath.name =       "Show visual keys";

        else if (parPath.name == "Visual keys text position offset")
        {
            newPath.name =       "Visual keys text position offset";
            newPath.collection = "[Buttons text advanced settings]";
        }

        else if (parPath.name == "Key counters text position offset")
        {
            newPath.name =       "Key counters text position offset";
            newPath.collection = "[Buttons text advanced settings]";
        }

        else if (parPath.name == "KPS text position offset")
        {
            newPath.name =       "KPS text position offset";
            newPath.collection = "[Buttons text advanced settings]";
        }

        else if (parPath.name == "BPM text position offset")
        {
            newPath.name =       "BPM text position offset";
            newPath.collection = "[Buttons text advanced settings]";
        }
    }

    else if (parPath.collection == "[Buttons text advanced settings]")
    {
        if (parPath.name == "Enable advanced mode for separate button value positions")
            newPath.name =  "Enable advanced mode for separate button text positions";
        else if (parPath.name == "Enable advanced mode for button text positions")
            newPath.name =       "Enable advanced mode for button text position";

        else if (parPath.name == "Visual keys text position offset")
            newPath.name =       "Buttons visual keys text position";

        else if (parPath.name == "Key counters text position offset")
            newPath.name =       "Buttons key counters text position";

        else if (parPath.name == "KPS text position offset")
            newPath.name =       "Buttons KPS text position";

        else if (parPath.name == "BPM text position offset")
            newPath.name =       "Buttons BPM text position";

        else if (const auto n = Utility::retrieveNumber(parPath.name, "@. Position offset"); 
            n != -1 && n <= static_cast<int>(Settings::OldSupportedAdvancedKeysNumber))
        {
            newPath.name = "Button text " + std::to_string(n) + " position";
        }
    }

    else if (parPath.collection == "[Button graphics]")
    {
        if (parPath.name == "Distance between buttons")
            newPath.name =  "Distance";
        else if (parPath.name == "Distance")
            newPath.name =       "Button distance";

        else if (parPath.name == "Texture filepath")
            newPath.name =       "Button texture";

        else if (parPath.name == "Texture size")
            newPath.name =       "Button texture size";

        else if (parPath.name == "Texture color")
            newPath.name =       "Button texture color";
    }

    else if (parPath.collection == "[Button graphics advanced settings]")
    {
        if (parPath.name == "Enable advanced mode for button graphics")
            newPath.name =  "Enable advanced mode for button positions";
        
        else if (const auto n = Utility::retrieveNumber(parPath.name, "@. Position offset"); 
            n != -1 && n <= static_cast<int>(Settings::OldSupportedAdvancedKeysNumber))
        {
            newPath.name = "Button " + std::to_string(n) + " position offset";
        }
        else if (const auto n = Utility::retrieveNumber(parPath.name, "Button @ position offset"); 
            n != -1 && n <= static_cast<int>(Settings::OldSupportedAdvancedKeysNumber))
        {
            newPath.name = "Button " + std::to_string(n) + " position";
        }

        else if (const auto n = Utility::retrieveNumber(parPath.name, "@. Size"); 
            n != -1 && n <= static_cast<int>(Settings::OldSupportedAdvancedKeysNumber))
        {
            newPath.name = "Button " + std::to_string(n) + " size";
        }
    }

    else if (parPath.collection == "[Common parameters]")
    {
        if (parPath.name == "Animation duration (frames)")
        {
            newPath.name =  "Animation frames";
            newPath.collection = "[Animation graphics]";
        }
    }

    else if (parPath.collection == "[Light animation]")
    {
        if (parPath.name == "Enabled")
            newPath.name =  "Light animation enabled";
        else if (parPath.name == "Light animation enabled")
        {
            newPath.name =       "Light animation";
            newPath.collection = "[Animation graphics]";
        }

        else if (parPath.name == "Texture filepath")
        {
            newPath.name =       "Animation texture";
            newPath.collection = "[Animation graphics]";
        }

        else if (parPath.name == "Scale on click (%)")
        {
            newPath.name =       "Animation scale on click (%)";
            newPath.collection = "[Animation graphics]";
        }

        else if (parPath.name == "Color")
        {
            newPath.name =       "Animation color";
            newPath.collection = "[Animation graphics]";
        }
    }

    else if (parPath.collection == "[Press animation]")
    {
        if (parPath.name == "Enabled")
            newPath.name =  "Press animation enabled";
        else if (parPath.name == "Press animation enabled")
        {
            newPath.name =       "Press animation";
            newPath.collection = "[Animation graphics]";
        }

        else if (parPath.name == "Offset")
        {
            newPath.name =       "Animation offset";
            newPath.collection = "[Animation graphics]";
        }
    }

    else if (parPath.collection == "[Main window]")
    {
        if (parPath.name == "Background texture filepath")
            newPath.name =  "Background texture";

        else if (parPath.name == "Title bar")
            newPath.name =       "Window title bar";

        else if (parPath.name == "Bonus size top")
            newPath.name =       "Window bonus size top";

        else if (parPath.name == "Bonus size bottom")
            newPath.name =       "Window bonus size bottom";

        else if (parPath.name == "Bonus size left")
            newPath.name =       "Window bonus size left";

        else if (parPath.name == "Bonus size right")
            newPath.name =       "Window bonus size right";
    }

    else if (parPath.collection == "[Extra KPS window]")
    {
        if (parPath.name == "Background color")
            newPath.name =  "KPS Background color";

        else if (parPath.name == "Text color")
            newPath.name =       "KPS text color";

        else if (parPath.name == "Number color")
            newPath.name =       "KPS number color";

        else if (parPath.name == "Text font filepath")
            newPath.name =       "KPS window text font";

        else if (parPath.name == "Number font filepaht")
            newPath.name =       "KPS window number font";

        else if (parPath.name == "Top padding")
            newPath.name =       "KPS top padding";

        else if (parPath.name == "Distance between text")
            newPath.name =       "KPS extra window distance between text";
    }

    else if (parPath.collection == "[Key press visualization]")
    {
        if (parPath.name == "Spawn position offset")
            newPath.name =  "Origin";
    }

    else if (parPath.collection == "[Key press visualization advanced settings]")
    {
    }

    else if (parPath.collection == "[Other]")
    {
        if (parPath.name == "Value to multiply on click")
            newPath.collection = "[Theme developer]";
    }

    if (newPath == parPath)
        newPath = ParameterPath();

    return newPath;
}