#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <cctype>
#include <curl/curl.h>
#include <fstream>
#include <set>

using namespace std;

// Callback function to write data received from curl
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

// Function to fetch the content of the web page
string fetchWebPage(const string &url)
{
    CURL *curl;
    CURLcode res;
    string data;

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if (res != CURLE_OK)
        {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            return "";
        }
    }
    return data;
}

// Function to remove HTML tags
string removeHTMLTags(const string &content)
{
    string result;
    bool insideTag = false;

    for (char ch : content)
    {
        if (ch == '<')
        {
            insideTag = true; // Start of an HTML tag
        }
        else if (ch == '>')
        {
            insideTag = false; // End of an HTML tag
        }
        else if (!insideTag)
        {
            result += ch; // Add characters outside of tags
        }
    }

    return result;
}

// Function to load the white list of words from the file
set<string> loadWhiteList(const string &filename)
{
    set<string> whiteList;
    ifstream file(filename);
    string word;

    if (!file.is_open())
    {
        cerr << "Failed to open file: " << filename << endl;
        return whiteList;
    }

    while (file >> word)
    {
        transform(word.begin(), word.end(), word.begin(), ::tolower); // Convert to lowercase
        whiteList.insert(word);
    }

    file.close();
    return whiteList;
}

// Function to process the web page content and count words based on the white list
void countWords(const string &content, map<string, int> &wordCount, const set<string> &whiteList)
{
    stringstream ss(content);
    string word;

    while (ss >> word)
    {
        // Remove punctuation and convert to lowercase
        word.erase(remove_if(word.begin(), word.end(), ::ispunct), word.end());
        transform(word.begin(), word.end(), word.begin(), ::tolower);
        if (!word.empty() && whiteList.find(word) != whiteList.end())
        { // Check if word is in white list
            wordCount[word]++;
        }
    }
}

// Function to display the word counts in ascending order (default)
void displayWordCounts(const map<string, int> &wordCount)
{
    for (const auto &pair : wordCount)
    {
        cout << pair.first << ": " << pair.second << endl;
    }
}

// Function to display the word counts in descending order by count
void displayWordCountsDescending(const map<string, int> &wordCount)
{
    // Create a vector of pairs (word, count) to store the data
    vector<pair<string, int>> sortedWords(wordCount.begin(), wordCount.end());

    // Sort the vector by the word count in descending order
    sort(sortedWords.begin(), sortedWords.end(), [](const pair<string, int> &a, const pair<string, int> &b)
         { return a.second > b.second; });

    // Display the sorted word counts
    for (const auto &pair : sortedWords)
    {
        cout << pair.first << ": " << pair.second << endl;
    }
}

int main()
{
    // Load the white list of words from the file
    set<string> whiteList = loadWhiteList("words.txt");
    if (whiteList.empty())
    {
        cerr << "White list is empty or could not be loaded." << endl;
        return 1;
    }

    string url;
    cout << "Enter the URL of the web page: ";
    getline(cin, url);

    string content = fetchWebPage(url);
    if (content.empty())
    {
        cerr << "Failed to fetch content from URL." << endl;
        return 1;
    }

    // Remove HTML tags from the fetched content
    string cleanContent = removeHTMLTags(content);

    // Word counting with white list
    map<string, int> wordCount;
    countWords(cleanContent, wordCount, whiteList);

    cout << "\nWord counts (ascending order):" << endl;
    displayWordCounts(wordCount);

    cout << "\nWord counts (descending order by frequency):" << endl;
    displayWordCountsDescending(wordCount);

    return 0;
}
