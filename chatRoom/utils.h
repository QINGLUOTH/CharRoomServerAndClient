#ifndef UTILS_H
#define UTILS_H

#include <map>
#include <vector>
#include <string>
#include <QDebug>

using namespace std;
//multimap
namespace util {
    static map<string, string>* analysisString(string AnalysisData) {
        // 解析数据
        map<string, string>* m1 = new map<string, string>;
        vector<string> v1;

        int last = -1;
        for (int i = 0; i < AnalysisData.length(); ++i) {
            if (AnalysisData[i] == ','){
                if (last == -1){
                    v1.push_back(AnalysisData.substr(0, i));
                }else {
                    v1.push_back(AnalysisData.substr(last + 1, i - (last + 1)));
                }
                last = i;
            }
        }
        v1.push_back(AnalysisData.substr(last + 1, AnalysisData.length() - last));

        for (int i = 0; i < v1.size(); ++i) {
            for (int j = 0; j < v1[i].size(); ++j) {
                if (v1[i][j] == ':'){
                    m1->insert(pair<string, string>(v1[i].substr(0, j), v1[i].substr(j + 1, v1[i].size())));
                    break;
                }
            }
        }
        return m1;
    }

    static multimap<string, string>* analysisStringByKeys(string AnalysisData) {
        // 解析数据
        multimap<string, string>* m1 = new multimap<string, string>;
        vector<string> v1;

        int last = -1;
        for (int i = 0; i < AnalysisData.length(); ++i) {
            if (AnalysisData[i] == ','){
                if (last == -1){
                    v1.push_back(AnalysisData.substr(0, i));
                }else {
                    v1.push_back(AnalysisData.substr(last + 1, i - (last + 1)));
                }
                last = i;
            }
        }
        v1.push_back(AnalysisData.substr(last + 1, AnalysisData.length() - last));

        for (int i = 0; i < v1.size(); ++i) {
            for (int j = 0; j < v1[i].size(); ++j) {
                if (v1[i][j] == ':'){
                    m1->insert(pair<string, string>(v1[i].substr(0, j), v1[i].substr(j + 1, v1[i].size())));
                    break;
                }
            }
        }
        return m1;
    }

    static vector<string>* analysisStringByKeysVector(string AnalysisData) {
        // 解析数据
        vector<string>* m1 = new vector<string>;
        vector<string> v1;

        int last = -1;
        for (int i = 0; i < AnalysisData.length(); ++i) {
            if (AnalysisData[i] == ','){
                if (last == -1){
                    v1.push_back(AnalysisData.substr(0, i));
                }else {
                    v1.push_back(AnalysisData.substr(last + 1, i - (last + 1)));
                }
                last = i;
            }
        }
        v1.push_back(AnalysisData.substr(last + 1, AnalysisData.length() - last));

        for (int i = 0; i < v1.size(); ++i) {
            for (int j = 0; j < v1[i].size(); ++j) {
                if (v1[i][j] == ':'){
                    string str = v1[i].substr(0, j);
                    str = str + ":\n\t" + v1[i].substr(j + 1, v1[i].size());
                    m1->push_back(str);
                    break;
                }
            }
        }
        return m1;
    }

    class Contact{
    public:
        string contactId;
        string userId;
        string messageId;
        string contactName;
        bool equals(Contact* contact){
            if (contactId == contact->contactId){
                // 表示这两个对象相等
                return true;
            }else {
                return false;
            }
        }

        bool operator==(Contact contact){
            return this->equals(&contact);
        }

        bool operator==(Contact* contact){
            return this->equals(contact);
        }

        bool equals(std::string contactId){
            if (contactId == this->contactId){
                // 表示这两个对象相等
                return true;
            }else {
                return false;
            }
        }
    };

    class Group{
    public:
        string groupId;
        string groupName;
        string userId;
        bool equals(Group* group){
            if (groupId == group->groupId){
                // 表示这两个对象相等
                return true;
            }else {
                return false;
            }
        }

        bool operator==(Group group){
            return this->equals(&group);
        }

        bool operator==(Group* group){
            return this->equals(group);
        }

        bool equals(std::string groupId){
            if (groupId == this->groupId){
                // 表示这两个对象相等
                return true;
            }else {
                return false;
            }
        }
    };


}

#endif // UTILS_H
