/* This file is part of node-sword-interface.

   Copyright (C) 2019 Tobias Klein <contact@ezra-project.net>

   node-sword-interface is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   node-sword-interface is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of 
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with node-sword-interface. See the file COPYING.
   If not, see <http://www.gnu.org/licenses/>. */


#include <swmodule.h>
#include "napi_sword_helper.hpp"
#include "sword_facade.hpp"

using namespace sword;

NapiSwordHelper::NapiSwordHelper() {
  this->_swordFacade = new SwordFacade();
}

NapiSwordHelper::~NapiSwordHelper() {
  delete this->_swordFacade;
}

Napi::Array NapiSwordHelper::getNapiVerseObjectsFromRawList(const Napi::Env& env, std::string& moduleCode, vector<string>& verses)
{
    Napi::Array versesArray = Napi::Array::New(env, verses.size());

    for (unsigned int i = 0; i < verses.size(); i++) {
        string currentRawVerse = verses[i];
        // FIXME: This only works within one bible book, not for the whole bible
        unsigned int currentAbsoluteVerseNr = i + 1;

        Napi::Object verseObject = Napi::Object::New(env);
        this->verseTextToNapiObject(moduleCode, currentRawVerse, currentAbsoluteVerseNr, verseObject);
        versesArray.Set(i, verseObject);
    }

    return versesArray;
}

void NapiSwordHelper::swordModuleToNapiObject(const Napi::Env& env, SWModule* swModule, Napi::Object& object)
{
    if (swModule == 0) {
        cerr << "swModule is 0! Cannot run conversion to napi object!" << endl;
        return;
    }

    object["name"] = swModule->getName();
    object["description"] = swModule->getDescription();
    object["language"] = swModule->getLanguage();
    object["version"] = swModule->getConfigEntry("Version");
    object["about"] = swModule->getConfigEntry("About");
    object["location"] = swModule->getConfigEntry("AbsoluteDataPath");
    
    bool moduleInUserDir = this->_swordFacade->isModuleInUserDir(swModule);
    object["inUserDir"] = Napi::Boolean::New(env, moduleInUserDir);

    if (swModule->getConfigEntry("Direction")) {
        string direction = string(swModule->getConfigEntry("Direction"));
        object["isRightToLeft"] = Napi::Boolean::New(env, (direction == "RtoL"));
    } else {
        object["isRightToLeft"] = Napi::Boolean::New(env, false);
    }

    bool moduleIsLocked = swModule->getConfigEntry("CipherKey");
    object["locked"] = Napi::Boolean::New(env, moduleIsLocked);

    if (swModule->getConfigEntry("InstallSize")) {
      int moduleSize = std::stoi(string(swModule->getConfigEntry("InstallSize")));
      object["size"] = Napi::Number::New(env, moduleSize);
    } else {
      object["size"] = Napi::Number::New(env, -1);
    }

    if (swModule->getConfigEntry("Abbreviation")) {
      object["abbreviation"] = swModule->getConfigEntry("Abbreviation");
    } else {
      object["abbreviation"] = "";
    }

    object["hasStrongs"] = Napi::Boolean::New(env, this->_swordFacade->moduleHasGlobalOption(swModule, "Strongs"));
    object["hasFootnotes"] = Napi::Boolean::New(env, this->_swordFacade->moduleHasGlobalOption(swModule, "Footnotes"));
    object["hasHeadings"] = Napi::Boolean::New(env, this->_swordFacade->moduleHasGlobalOption(swModule, "Headings"));
    object["hasRedLetterWords"] = Napi::Boolean::New(env, this->_swordFacade->moduleHasGlobalOption(swModule, "RedLetter"));
    object["hasCrossReferences"] = Napi::Boolean::New(env, this->_swordFacade->moduleHasGlobalOption(swModule, "Scripref"));
}

void NapiSwordHelper::verseTextToNapiObject(std::string& moduleCode, string& rawVerse, unsigned int absoluteVerseNr, Napi::Object& object)
{
    vector<string> splittedVerse = this->split(rawVerse, '|');
    string reference = splittedVerse[0];
    string verseText = splittedVerse[1];

    vector<string> splittedReference = this->split(reference, ' ');
    string book = splittedReference[0];
    string chapterVerseReference = splittedReference[1];

    vector<string> splittedChapterVerseReference = this->split(chapterVerseReference, ':');
    string chapter = splittedChapterVerseReference[0];
    string verseNr = splittedChapterVerseReference[1];

    object["moduleCode"] = moduleCode;
    object["bibleBookShortTitle"] = book;
    object["chapter"] = chapter;
    object["verseNr"] = verseNr;
    object["absoluteVerseNr"] = absoluteVerseNr;
    object["content"] = verseText;
}

vector<string> NapiSwordHelper::split(const string& s, char separator)
{
    vector<string> output;
    string::size_type prev_pos = 0, pos = 0;

    while((pos = s.find(separator, pos)) != string::npos)
    {
        string substring( s.substr(prev_pos, pos-prev_pos) );
        output.push_back(substring);
        prev_pos = ++pos;
    }

    output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word
    return output;
}