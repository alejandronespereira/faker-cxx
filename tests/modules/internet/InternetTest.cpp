#include "faker-cxx/Internet.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "gtest/gtest.h"

#include "common/StringHelper.h"
#include "faker-cxx/Number.h"
#include "faker-cxx/types/Country.h"
#include "internet/InternetData.h"
#include "person/PersonData.h"
#include "string/StringData.h"
#include "word/WordData.h"

using namespace ::testing;
using namespace faker;

namespace
{
const std::string passwordCharacters =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789~`!@#$%^&*()_-+={[}]|:;\"'<,>.?/";
const std::initializer_list<std::string_view> webProtocols{"http", "https"};
const std::initializer_list<std::string_view> httpMethodNames{"GET", "POST", "DELETE", "PATCH", "PUT"};
const std::initializer_list<unsigned> httpStatusInformationalCodes{100, 101, 102, 103};
const std::initializer_list<unsigned> httpStatusSuccessCodes{200, 201, 202, 203, 204, 205, 206, 207, 208, 226};
const std::initializer_list<unsigned> httpStatusRedirectionCodes{300, 301, 302, 303, 304, 305, 306, 307, 308};
const std::initializer_list<unsigned> httpStatusClientErrorCodes{400, 401, 402, 403, 404, 405, 406, 407, 408, 409,
                                                                 410, 411, 412, 413, 414, 415, 416, 417, 418, 421,
                                                                 422, 423, 424, 425, 426, 428, 429, 431, 451};
const std::initializer_list<unsigned> httpStatusServerErrorCodes{500, 501, 502, 503, 504, 505, 506, 507, 508, 510, 511};
constexpr unsigned int classAFirstSection = 10u;
constexpr unsigned int classBFirstSection = 172u;
constexpr unsigned int classBSecondSectionLowerBound = 16;
constexpr unsigned int classBSecondSectionUpperBound = 31;
constexpr unsigned int classCFirstSection = 192u;
constexpr unsigned int classCSecondSection = 168u;

std::array<unsigned int, 4> deconstructIpv4String(const std::string& ipv4)
{
    const auto ipParts = StringHelper::split(ipv4, ".");

    std::array<unsigned int, 4> result{static_cast<unsigned>(stoi(ipParts[0])), static_cast<unsigned>(stoi(ipParts[1])),
                                       static_cast<unsigned>(stoi(ipParts[2])),
                                       static_cast<unsigned>(stoi(ipParts[3]))};
    return result;
}
}

class InternetTest : public Test
{
public:
    InternetTest()
    {
        sortedAdjectivesDescendingBySize.assign(word::adjectives.cbegin(), word::adjectives.cend());
        std::sort(sortedAdjectivesDescendingBySize.begin(), sortedAdjectivesDescendingBySize.end(),
                  [](const std::string& first, const std::string& second) { return first.size() > second.size(); });
    }

    void assertDomainWord(const std::string& domainWord)
    {
        std::optional<std::string> foundAdjective = std::nullopt;

        for (const auto& adjective : sortedAdjectivesDescendingBySize)
        {
            if (domainWord.find(adjective) == 0)
            {
                foundAdjective = adjective;
                break;
            }
        }

        ASSERT_TRUE(foundAdjective);

        ASSERT_EQ(domainWord[foundAdjective->size()], '-');

        const auto generatedNoun = domainWord.substr(foundAdjective->size() + 1);

        ASSERT_TRUE(std::ranges::any_of(word::nouns,
                                        [generatedNoun](const std::string_view& noun)
                                        {
                                            if (generatedNoun.size() != noun.size())
                                                return false;
                                            for (size_t i = 0; i < noun.size(); i++)
                                                if (std::tolower(noun[i]) != std::tolower(generatedNoun[i]))
                                                    return false;
                                            return true;
                                        }));
    }

    std::vector<std::string> sortedAdjectivesDescendingBySize;
};

TEST_F(InternetTest, shouldGenerateUsername)
{
    std::vector<std::string_view> firstNames(englishMaleFirstNames.begin(), englishMaleFirstNames.end());
    firstNames.insert(firstNames.end(), englishFemaleFirstNames.begin(), englishFemaleFirstNames.end());

    const auto username = Internet::username();

    ASSERT_TRUE(std::ranges::any_of(firstNames, [username](const std::string_view& firstName)
                                    { return username.find(firstName) != std::string::npos; }));
    ASSERT_TRUE(std::ranges::any_of(englishLastNames, [username](const std::string_view& lastName)
                                    { return username.find(lastName) != std::string::npos; }));
}

TEST_F(InternetTest, shouldGenerateUsernameWithFirstNameProvided)
{
    const auto firstName = "Michael";

    const auto username = Internet::username(firstName);

    ASSERT_TRUE(username.find(firstName) != std::string::npos);
    ASSERT_TRUE(std::ranges::any_of(englishLastNames, [username](const std::string_view& lastName)
                                    { return username.find(lastName) != std::string::npos; }));
}

TEST_F(InternetTest, shouldGenerateUsernameWithLastNameProvided)
{
    std::vector<std::string_view> firstNames(englishMaleFirstNames.begin(), englishMaleFirstNames.end());
    firstNames.insert(firstNames.end(), englishFemaleFirstNames.begin(), englishFemaleFirstNames.end());

    const auto lastName = "Cieslar";

    const auto username = Internet::username(std::nullopt, lastName);

    ASSERT_TRUE(std::ranges::any_of(firstNames, [username](const std::string_view& firstName)
                                    { return username.find(firstName) != std::string::npos; }));
    ASSERT_TRUE(username.find(lastName) != std::string::npos);
}

TEST_F(InternetTest, shouldGenerateUsernameWithFullNameProvided)
{
    const auto firstName = "Andrew";

    const auto lastName = "Cieslar";

    const auto username = Internet::username(firstName, lastName);

    ASSERT_TRUE(username.find(firstName) != std::string::npos);
    ASSERT_TRUE(username.find(lastName) != std::string::npos);
}

TEST_F(InternetTest, shouldGenerateInternationalUsernames)
{
    std::vector<std::string_view> firstNames(polishMaleFirstNames.begin(), polishMaleFirstNames.end());
    firstNames.insert(firstNames.end(), polishFemaleFirstNames.begin(), polishFemaleFirstNames.end());

    const auto username = Internet::username(std::nullopt, std::nullopt, Country::Poland);

    ASSERT_TRUE(std::ranges::any_of(firstNames, [username](const std::string_view& firstName)
                                    { return username.find(firstName) != std::string::npos; }));

    ASSERT_TRUE(std::ranges::any_of(polishLastNames, [username](const std::string_view& lastName)
                                    { return username.find(lastName) != std::string::npos; }));
}

TEST_F(InternetTest, shouldGenerateEmail)
{
    std::vector<std::string_view> firstNames(englishMaleFirstNames.begin(), englishMaleFirstNames.end());
    firstNames.insert(firstNames.end(), englishFemaleFirstNames.begin(), englishFemaleFirstNames.end());

    const auto email = Internet::email();

    const auto emailParts = StringHelper::split(email, "@");

    ASSERT_EQ(emailParts.size(), 2);

    const auto& generatedUsername = emailParts[0];
    const auto& generatedEmailHost = emailParts[1];

    ASSERT_TRUE(std::ranges::any_of(emailHosts, [generatedEmailHost](const std::string_view& emailHost)
                                    { return generatedEmailHost == emailHost; }));
    ASSERT_TRUE(std::ranges::any_of(firstNames, [generatedUsername](const std::string_view& firstName)
                                    { return generatedUsername.find(firstName) != std::string::npos; }));
    ASSERT_TRUE(std::ranges::any_of(englishLastNames, [generatedUsername](const std::string_view& lastName)
                                    { return generatedUsername.find(lastName) != std::string::npos; }));
}

TEST_F(InternetTest, shouldGenerateEmailWithFirstName)
{
    const auto firstName = "Tom";

    const auto email = Internet::email(firstName);

    const auto emailParts = StringHelper::split(email, "@");

    ASSERT_EQ(emailParts.size(), 2);

    const auto& generatedUsername = emailParts[0];
    const auto& generatedEmailHost = emailParts[1];

    ASSERT_TRUE(std::ranges::any_of(emailHosts, [generatedEmailHost](const std::string_view& emailHost)
                                    { return generatedEmailHost == emailHost; }));
    ASSERT_TRUE(generatedUsername.find(firstName) != std::string::npos);
    ASSERT_TRUE(std::ranges::any_of(englishLastNames, [generatedUsername](const std::string_view& lastName)
                                    { return generatedUsername.find(lastName) != std::string::npos; }));
}

TEST_F(InternetTest, shouldGenerateEmailWithLastName)
{
    std::vector<std::string_view> firstNames(englishMaleFirstNames.begin(), englishMaleFirstNames.end());
    firstNames.insert(firstNames.end(), englishFemaleFirstNames.begin(), englishFemaleFirstNames.end());

    const auto lastName = "Howard";

    const auto email = Internet::email(std::nullopt, lastName);

    const auto emailParts = StringHelper::split(email, "@");

    ASSERT_EQ(emailParts.size(), 2);

    const auto& generatedUsername = emailParts[0];
    const auto& generatedEmailHost = emailParts[1];

    ASSERT_TRUE(std::ranges::any_of(emailHosts, [generatedEmailHost](const std::string_view& emailHost)
                                    { return generatedEmailHost == emailHost; }));
    ASSERT_TRUE(std::ranges::any_of(firstNames, [generatedUsername](const std::string_view& firstName)
                                    { return generatedUsername.find(firstName) != std::string::npos; }));
    ASSERT_TRUE(generatedUsername.find(lastName) != std::string::npos);
}

TEST_F(InternetTest, shouldGenerateEmailWithFullName)
{
    const auto firstName = "Cindy";

    const auto lastName = "Young";

    const auto email = Internet::email(firstName, lastName);

    const auto emailParts = StringHelper::split(email, "@");

    ASSERT_EQ(emailParts.size(), 2);

    const auto& generatedUsername = emailParts[0];
    const auto& generatedEmailHost = emailParts[1];

    ASSERT_TRUE(std::ranges::any_of(emailHosts, [generatedEmailHost](const std::string_view& emailHost)
                                    { return generatedEmailHost == emailHost; }));
    ASSERT_TRUE(generatedUsername.find(firstName) != std::string::npos);
    ASSERT_TRUE(generatedUsername.find(lastName) != std::string::npos);
}

TEST_F(InternetTest, shouldGenerateEmailWithSpecifiedEmailHost)
{
    std::vector<std::string_view> firstNames(englishMaleFirstNames.begin(), englishMaleFirstNames.end());
    firstNames.insert(firstNames.end(), englishFemaleFirstNames.begin(), englishFemaleFirstNames.end());

    const auto emailHost = "example.com";

    const auto email = Internet::email(std::nullopt, std::nullopt, emailHost);

    const auto emailParts = StringHelper::split(email, "@");

    ASSERT_EQ(emailParts.size(), 2);

    const auto& generatedUsername = emailParts[0];
    const auto& generatedEmailHost = emailParts[1];

    ASSERT_EQ(generatedEmailHost, emailHost);
    ASSERT_TRUE(std::ranges::any_of(firstNames, [generatedUsername](const std::string_view& firstName)
                                    { return generatedUsername.find(firstName) != std::string::npos; }));
    ASSERT_TRUE(std::ranges::any_of(englishLastNames, [generatedUsername](const std::string_view& lastName)
                                    { return generatedUsername.find(lastName) != std::string::npos; }));
}

TEST_F(InternetTest, shouldGenerateExampleEmail)
{
    std::vector<std::string_view> firstNames(englishMaleFirstNames.begin(), englishMaleFirstNames.end());
    firstNames.insert(firstNames.end(), englishFemaleFirstNames.begin(), englishFemaleFirstNames.end());

    const auto email = Internet::exampleEmail();

    const auto emailParts = StringHelper::split(email, "@");

    ASSERT_EQ(emailParts.size(), 2);

    const auto& generatedUsername = emailParts[0];
    const auto& generatedEmailHost = emailParts[1];

    ASSERT_TRUE(std::ranges::any_of(emailExampleHosts, [generatedEmailHost](const std::string_view& emailHost)
                                    { return generatedEmailHost == emailHost; }));
    ASSERT_TRUE(std::ranges::any_of(firstNames, [generatedUsername](const std::string_view& firstName)
                                    { return generatedUsername.find(firstName) != std::string::npos; }));
    ASSERT_TRUE(std::ranges::any_of(englishLastNames, [generatedUsername](const std::string_view& lastName)
                                    { return generatedUsername.find(lastName) != std::string::npos; }));
}

TEST_F(InternetTest, shouldGenerateExampleEmailWithFirstName)
{
    const auto firstName = "Barry";

    const auto email = Internet::exampleEmail(firstName);

    const auto emailParts = StringHelper::split(email, "@");

    ASSERT_EQ(emailParts.size(), 2);

    const auto& generatedUsername = emailParts[0];
    const auto& generatedEmailHost = emailParts[1];

    ASSERT_TRUE(std::ranges::any_of(emailExampleHosts, [generatedEmailHost](const std::string_view& emailHost)
                                    { return generatedEmailHost == emailHost; }));
    ASSERT_TRUE(generatedUsername.find(firstName) != std::string::npos);
    ASSERT_TRUE(std::ranges::any_of(englishLastNames, [generatedUsername](const std::string_view& lastName)
                                    { return generatedUsername.find(lastName) != std::string::npos; }));
}

TEST_F(InternetTest, shouldGenerateExampleEmailWithLastName)
{
    std::vector<std::string_view> firstNames(englishMaleFirstNames.begin(), englishMaleFirstNames.end());
    firstNames.insert(firstNames.end(), englishFemaleFirstNames.begin(), englishFemaleFirstNames.end());

    const auto lastName = "Wilkinson";

    const auto email = Internet::exampleEmail(std::nullopt, lastName);

    const auto emailParts = StringHelper::split(email, "@");

    ASSERT_EQ(emailParts.size(), 2);

    const auto& generatedUsername = emailParts[0];
    const auto& generatedEmailHost = emailParts[1];

    ASSERT_TRUE(std::ranges::any_of(emailExampleHosts, [generatedEmailHost](const std::string_view& emailHost)
                                    { return generatedEmailHost == emailHost; }));
    ASSERT_TRUE(std::ranges::any_of(firstNames, [generatedUsername](const std::string_view& firstName)
                                    { return generatedUsername.find(firstName) != std::string::npos; }));
    ASSERT_TRUE(generatedUsername.find(lastName) != std::string::npos);
}

TEST_F(InternetTest, shouldGenerateExampleEmailWithFullName)
{
    const auto firstName = "Walter";

    const auto lastName = "Brown";

    const auto email = Internet::exampleEmail(firstName, lastName);

    const auto emailParts = StringHelper::split(email, "@");

    ASSERT_EQ(emailParts.size(), 2);

    const auto& generatedUsername = emailParts[0];
    const auto& generatedEmailHost = emailParts[1];

    ASSERT_TRUE(std::ranges::any_of(emailExampleHosts, [generatedEmailHost](const std::string_view& emailHost)
                                    { return generatedEmailHost == emailHost; }));
    ASSERT_TRUE(generatedUsername.find(firstName) != std::string::npos);
    ASSERT_TRUE(generatedUsername.find(lastName) != std::string::npos);
}

TEST_F(InternetTest, shouldGeneratePassword)
{
    const auto password = Internet::password();

    ASSERT_EQ(password.size(), 15);
    ASSERT_TRUE(std::ranges::all_of(password, [&](char passwordCharacter)
                                    { return passwordCharacters.find(passwordCharacter) != std::string::npos; }));
}

TEST_F(InternetTest, shouldGeneratePasswordWithSpecifiedLength)
{
    const auto passwordLength = 25;

    const auto password = Internet::password(passwordLength);

    ASSERT_EQ(password.size(), passwordLength);
    ASSERT_TRUE(std::ranges::all_of(password, [&](char passwordCharacter)
                                    { return passwordCharacters.find(passwordCharacter) != std::string::npos; }));
}

TEST_F(InternetTest, shouldGenerateEmoji)
{
    const auto generatedEmoji = Internet::emoji();

    std::vector<std::string_view> emojis;
    emojis.reserve(smileyEmojis.size() + bodyEmojis.size() + personEmojis.size() + natureEmojis.size() +
                   foodEmojis.size() + travelEmojis.size() + activityEmojis.size() + objectEmojis.size() +
                   symbolEmojis.size() + flagEmojis.size());

    emojis.insert(emojis.end(), smileyEmojis.begin(), smileyEmojis.end());
    emojis.insert(emojis.end(), bodyEmojis.begin(), bodyEmojis.end());
    emojis.insert(emojis.end(), personEmojis.begin(), personEmojis.end());
    emojis.insert(emojis.end(), natureEmojis.begin(), natureEmojis.end());
    emojis.insert(emojis.end(), foodEmojis.begin(), foodEmojis.end());
    emojis.insert(emojis.end(), travelEmojis.begin(), travelEmojis.end());
    emojis.insert(emojis.end(), activityEmojis.begin(), activityEmojis.end());
    emojis.insert(emojis.end(), objectEmojis.begin(), objectEmojis.end());
    emojis.insert(emojis.end(), symbolEmojis.begin(), symbolEmojis.end());
    emojis.insert(emojis.end(), flagEmojis.begin(), flagEmojis.end());

    ASSERT_TRUE(std::ranges::any_of(emojis, [generatedEmoji](const std::string_view& emoji)
                                    { return generatedEmoji == emoji; }));
}

TEST_F(InternetTest, shouldGenerateSmileyEmoji)
{
    const auto generatedEmoji = Internet::emoji(Internet::EmojiType::Smiley);

    ASSERT_TRUE(std::ranges::any_of(smileyEmojis, [generatedEmoji](const std::string_view& emoji)
                                    { return generatedEmoji == emoji; }));
}

TEST_F(InternetTest, shouldGenerateBodyEmoji)
{
    const auto generatedEmoji = Internet::emoji(Internet::EmojiType::Body);

    ASSERT_TRUE(std::ranges::any_of(bodyEmojis, [generatedEmoji](const std::string_view& emoji)
                                    { return generatedEmoji == emoji; }));
}

TEST_F(InternetTest, shouldGeneratePersonEmoji)
{
    const auto generatedEmoji = Internet::emoji(Internet::EmojiType::Person);

    ASSERT_TRUE(std::ranges::any_of(personEmojis, [generatedEmoji](const std::string_view& emoji)
                                    { return generatedEmoji == emoji; }));
}

TEST_F(InternetTest, shouldGenerateNatureEmoji)
{
    const auto generatedEmoji = Internet::emoji(Internet::EmojiType::Nature);

    ASSERT_TRUE(std::ranges::any_of(natureEmojis, [generatedEmoji](const std::string_view& emoji)
                                    { return generatedEmoji == emoji; }));
}

TEST_F(InternetTest, shouldGenerateFoodEmoji)
{
    const auto generatedEmoji = Internet::emoji(Internet::EmojiType::Food);

    ASSERT_TRUE(std::ranges::any_of(foodEmojis, [generatedEmoji](const std::string_view& emoji)
                                    { return generatedEmoji == emoji; }));
}

TEST_F(InternetTest, shouldGenerateTravelEmoji)
{
    const auto generatedEmoji = Internet::emoji(Internet::EmojiType::Travel);

    ASSERT_TRUE(std::ranges::any_of(travelEmojis, [generatedEmoji](const std::string_view& emoji)
                                    { return generatedEmoji == emoji; }));
}

TEST_F(InternetTest, shouldGenerateActivityEmoji)
{
    const auto generatedEmoji = Internet::emoji(Internet::EmojiType::Activity);

    ASSERT_TRUE(std::ranges::any_of(activityEmojis, [generatedEmoji](const std::string_view& emoji)
                                    { return generatedEmoji == emoji; }));
}

TEST_F(InternetTest, shouldGenerateObjectEmoji)
{
    const auto generatedEmoji = Internet::emoji(Internet::EmojiType::Object);

    ASSERT_TRUE(std::ranges::any_of(objectEmojis, [generatedEmoji](const std::string_view& emoji)
                                    { return generatedEmoji == emoji; }));
}

TEST_F(InternetTest, shouldGenerateSymbolEmoji)
{
    const auto generatedEmoji = Internet::emoji(Internet::EmojiType::Symbol);

    ASSERT_TRUE(std::ranges::any_of(symbolEmojis, [generatedEmoji](const std::string_view& emoji)
                                    { return generatedEmoji == emoji; }));
}

TEST_F(InternetTest, shouldGenerateFlagEmoji)
{
    const auto generatedEmoji = Internet::emoji(Internet::EmojiType::Flag);

    ASSERT_TRUE(std::ranges::any_of(flagEmojis, [generatedEmoji](const std::string_view& emoji)
                                    { return generatedEmoji == emoji; }));
}

TEST_F(InternetTest, shouldGenerateProtocol)
{
    const auto webProtocol = Internet::protocol();

    ASSERT_TRUE(std::ranges::any_of(webProtocols, [webProtocol](const std::string_view& protocol)
                                    { return webProtocol == protocol; }));
}

TEST_F(InternetTest, shouldGenerateHttpMethod)
{
    const auto generatedHttpMethod = Internet::httpMethod();

    ASSERT_TRUE(std::ranges::any_of(httpMethodNames, [generatedHttpMethod](const std::string_view& httpMethod)
                                    { return generatedHttpMethod == httpMethod; }));
}

TEST_F(InternetTest, shouldGenerateHttpStatusCode)
{
    const auto generatedHttpStatusCode = Internet::httpStatusCode();

    std::vector<unsigned> statusCodes;
    statusCodes.insert(statusCodes.end(), httpStatusInformationalCodes.begin(), httpStatusInformationalCodes.end());
    statusCodes.insert(statusCodes.end(), httpStatusSuccessCodes.begin(), httpStatusSuccessCodes.end());
    statusCodes.insert(statusCodes.end(), httpStatusRedirectionCodes.begin(), httpStatusRedirectionCodes.end());
    statusCodes.insert(statusCodes.end(), httpStatusClientErrorCodes.begin(), httpStatusClientErrorCodes.end());
    statusCodes.insert(statusCodes.end(), httpStatusServerErrorCodes.begin(), httpStatusServerErrorCodes.end());

    ASSERT_TRUE(std::ranges::any_of(statusCodes, [generatedHttpStatusCode](unsigned statusCode)
                                    { return generatedHttpStatusCode == statusCode; }));
}

TEST_F(InternetTest, shouldGenerateHttpRequestHeader)
{
    const auto generatedHttpRequestHeader = Internet::httpRequestHeader();

    ASSERT_TRUE(std::ranges::any_of(httpRequestHeaders, [generatedHttpRequestHeader](const std::string_view& httpHeader)
                                    { return generatedHttpRequestHeader == httpHeader; }));
}

TEST_F(InternetTest, shouldGenerateHttpResponseHeader)
{
    const auto generatedHttpResponseHeader = Internet::httpResponseHeader();

    ASSERT_TRUE(std::ranges::any_of(httpResponseHeaders,
                                    [generatedHttpResponseHeader](const std::string_view& httpHeader)
                                    { return generatedHttpResponseHeader == httpHeader; }));
}

TEST_F(InternetTest, shouldGenerateHttpMediaType)
{
    const auto generatedHttpMediaType = Internet::httpMediaType();

    ASSERT_TRUE(std::ranges::any_of(httpMediaTypes, [generatedHttpMediaType](const std::string_view& httpMediaType)
                                    { return generatedHttpMediaType == httpMediaType; }));
}

TEST_F(InternetTest, shouldGenerateHttpInformationalSuccessCode)
{
    const auto generatedHttpStatusCode = Internet::httpStatusCode(HttpResponseType::Informational);

    ASSERT_TRUE(std::ranges::any_of(httpStatusInformationalCodes, [generatedHttpStatusCode](unsigned statusCode)
                                    { return generatedHttpStatusCode == statusCode; }));
}

TEST_F(InternetTest, shouldGenerateHttpStatusSuccessCode)
{
    const auto generatedHttpStatusCode = Internet::httpStatusCode(HttpResponseType::Success);

    ASSERT_TRUE(std::ranges::any_of(httpStatusSuccessCodes, [generatedHttpStatusCode](unsigned statusCode)
                                    { return generatedHttpStatusCode == statusCode; }));
}

TEST_F(InternetTest, shouldGenerateHttpStatusRedirectionCode)
{
    const auto generatedHttpStatusCode = Internet::httpStatusCode(HttpResponseType::Redirection);

    ASSERT_TRUE(std::ranges::any_of(httpStatusRedirectionCodes, [generatedHttpStatusCode](unsigned statusCode)
                                    { return generatedHttpStatusCode == statusCode; }));
}

TEST_F(InternetTest, shouldGenerateHttpStatusClientErrorCode)
{
    const auto generatedHttpStatusCode = Internet::httpStatusCode(HttpResponseType::ClientError);

    ASSERT_TRUE(std::ranges::any_of(httpStatusClientErrorCodes, [generatedHttpStatusCode](unsigned statusCode)
                                    { return generatedHttpStatusCode == statusCode; }));
}

TEST_F(InternetTest, shouldGenerateHttpStatusServerErrorCode)
{
    const auto generatedHttpStatusCode = Internet::httpStatusCode(HttpResponseType::ServerError);

    ASSERT_TRUE(std::ranges::any_of(httpStatusServerErrorCodes, [generatedHttpStatusCode](unsigned statusCode)
                                    { return generatedHttpStatusCode == statusCode; }));
}

TEST_F(InternetTest, shouldGenerateIpv4WithPrivateClassAAddress)
{
    const auto generatedIpv4 = Internet::ipv4(IPv4Class::A);
    const auto addressSectors = deconstructIpv4String(generatedIpv4);

    ASSERT_EQ(addressSectors[0], classAFirstSection);
}

TEST_F(InternetTest, shouldGenerateIpv4WithPrivateClassBAddress)
{
    const auto generatedIpv4 = Internet::ipv4(IPv4Class::B);

    const auto addressSectors = deconstructIpv4String(generatedIpv4);

    ASSERT_EQ(addressSectors[0], classBFirstSection);
    ASSERT_TRUE(addressSectors[1] >= classBSecondSectionLowerBound and
                addressSectors[1] <= classBSecondSectionUpperBound);
}

TEST_F(InternetTest, shouldGenerateIpv4WithPrivateClassCAddress)
{
    const auto generatedIpv4 = Internet::ipv4(IPv4Class::C);
    const auto addressSectors = deconstructIpv4String(generatedIpv4);

    ASSERT_EQ(addressSectors[0], classCFirstSection);
    ASSERT_EQ(addressSectors[1], classCSecondSection);
}

TEST_F(InternetTest, shouldGenerateIpv4KeepingTheMaskedPart)
{
    const std::array<unsigned int, 4> sampleAddress = {192, 168, 10, 12};
    const std::array<unsigned int, 4> generationMask = {255, 128, 0, 0};

    const auto generatedAddress = deconstructIpv4String(Internet::ipv4(sampleAddress, generationMask));

    constexpr unsigned int expectedSecondSectorMaskedValue = 0x00000080;

    ASSERT_EQ(sampleAddress[0], generatedAddress[0]);
    ASSERT_TRUE((generatedAddress[1] & generationMask[1]) == expectedSecondSectorMaskedValue);
}

TEST_F(InternetTest, shouldGenerateIpv6)
{
    const auto generatedIpv6 = Internet::ipv6();

    const auto generatedIpv6Parts = StringHelper::split(generatedIpv6, ":");

    ASSERT_EQ(generatedIpv6Parts.size(), 8);

    ASSERT_TRUE(std::ranges::all_of(generatedIpv6Parts, [](const std::string& generatedIpv6Part)
                                    { return generatedIpv6Part.size() == 4; }));
    ASSERT_TRUE(std::ranges::all_of(generatedIpv6Parts,
                                    [](const std::string_view& generatedIpv6Part)
                                    {
                                        return std::ranges::all_of(
                                            generatedIpv6Part, [](char hexCharacter)
                                            { return hexLowerCharacters.find(hexCharacter) != std::string::npos; });
                                    }));
}

TEST_F(InternetTest, MacDefaultSeparator)
{
    const auto mac = Internet::mac();

    ASSERT_EQ(mac.size(), 17);

    for (size_t i = 0; i < mac.size(); i += 3)
    {
        ASSERT_TRUE(isxdigit(mac[i]));
    }

    for (size_t i = 2; i < mac.size(); i += 3)
    {
        ASSERT_EQ(mac[i], ':');
    }
}

TEST_F(InternetTest, shouldGenerateDomainSuffix)
{
    const auto generatedDomainSuffix = Internet::domainSuffix();

    ASSERT_TRUE(std::ranges::any_of(domainSuffixes, [generatedDomainSuffix](const std::string_view& domainSuffix)
                                    { return generatedDomainSuffix == domainSuffix; }));
}

TEST_F(InternetTest, shouldGenerateDomainWord)
{
    const auto generatedDomainWord = Internet::domainWord();

    assertDomainWord(generatedDomainWord);
}

TEST_F(InternetTest, shouldGenerateDomainName)
{
    const auto generatedDomainName = Internet::domainName();

    const auto generatedDomainNameParts = StringHelper::split(generatedDomainName, ".");

    const auto& generatedDomainWord = generatedDomainNameParts[0];
    const auto& generatedDomainSuffix = generatedDomainNameParts[1];

    assertDomainWord(generatedDomainWord);
    ASSERT_TRUE(std::ranges::any_of(domainSuffixes, [generatedDomainSuffix](const std::string_view& domainSuffix)
                                    { return generatedDomainSuffix == domainSuffix; }));
}

TEST_F(InternetTest, shouldGenerateHttpsUrl)
{
    const auto generatedUrl = Internet::url();

    const auto generatedUrlParts = StringHelper::split(generatedUrl, "://");

    const auto& generatedProtocol = generatedUrlParts[0];
    const auto& generatedDomainName = generatedUrlParts[1];

    const auto generatedDomainNameParts = StringHelper::split(generatedDomainName, ".");

    const auto& generatedDomainWord = generatedDomainNameParts[0];
    const auto& generatedDomainSuffix = generatedDomainNameParts[1];

    assertDomainWord(generatedDomainWord);
    ASSERT_TRUE(std::ranges::any_of(domainSuffixes, [generatedDomainSuffix](const std::string_view& domainSuffix)
                                    { return generatedDomainSuffix == domainSuffix; }));
    ASSERT_EQ(generatedProtocol, "https");
}

TEST_F(InternetTest, shouldGenerateHttpUrl)
{
    const auto generatedUrl = Internet::url(WebProtocol::Http);

    const auto generatedUrlParts = StringHelper::split(generatedUrl, "://");

    const auto& generatedProtocol = generatedUrlParts[0];
    const auto& generatedDomainName = generatedUrlParts[1];

    const auto generatedDomainNameParts = StringHelper::split(generatedDomainName, ".");

    const auto& generatedDomainWord = generatedDomainNameParts[0];
    const auto& generatedDomainSuffix = generatedDomainNameParts[1];

    assertDomainWord(generatedDomainWord);
    ASSERT_TRUE(std::ranges::any_of(domainSuffixes, [generatedDomainSuffix](const std::string_view& domainSuffix)
                                    { return generatedDomainSuffix == domainSuffix; }));
    ASSERT_EQ(generatedProtocol, "http");
}

TEST_F(InternetTest, shouldGeneratePort)
{
    const auto generatedPort = Internet::port();

    ASSERT_GE(generatedPort, 0);
    ASSERT_LE(generatedPort, 65535);
}

TEST_F(InternetTest, shouldGenerateAnonymousUsername)
{
    for (int i = 0; i < 100; i++)
    {
        const std::integral auto maxLength = number::integer<unsigned>(6, 20);
        const auto generatedUsername = Internet::anonymousUsername(maxLength);

        ASSERT_EQ(generatedUsername.length(), maxLength);
    }
}

TEST_F(InternetTest, shouldGenerateAnonymousUsernameWithMinLength)
{
    const auto maxLength = 5;
    const auto generatedUsername = Internet::anonymousUsername(maxLength);

    ASSERT_EQ(generatedUsername.length(), 6);
}

TEST_F(InternetTest, shouldGenerateAnonymousUsernameWithMaxLength)
{
    const auto maxLength = 21;
    const auto generatedUsername = Internet::anonymousUsername(maxLength);

    ASSERT_EQ(generatedUsername.length(), 20);
}
