#include <iostream>
#include <fstream>
#include <picojson.h>
#include <tclap/CmdLine.h>
#include <regex>
#include <set>

namespace util
{
    std::string ToString(int number)
    {
        std::ostringstream stringStream;
        stringStream << number;
        return stringStream.str();
    }

    void ReplaceAll(std::string& str, const std::string& from, const std::string& to)
    {
        if (from.empty())
        {
            return;
        }

        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos)
        {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }
}

namespace converter
{
    typedef std::pair<std::string, std::string> ValuePair;
    class PropertyNamespace;
    class Rules;

    void WriteProperty(PropertyNamespace & currentNamespace, std::string const & name, std::string & value, std::ofstream & stream, Rules const & rules);

    class PropertyNamespace
    {
    public:
        enum class Modification
        {
            None,
            ValueAdded,
            NamespaceAdded
        };

        PropertyNamespace(picojson::value & json)
            : depth(-1)
            , previousModification(Modification::None)
            , skippedNamespaceCount(0)
            , json(json)
        {
        }

        PropertyNamespace(picojson::value & json, std::string const & nameArg, int depth, PropertyNamespace const & parent)
            : name(nameArg.empty() ? (parent.GetName() + "_" + util::ToString(parent.GetNamespaceCount() + parent.GetSkippedNamespaceCount())) : nameArg)
            , depth(depth)
            , previousModification(Modification::None)
            , path(parent.GetPath() + (name.empty() ? "" : "/" + name))
            , skippedNamespaceCount(0)
            , json(json)
        {
        }

        void AddNamespace(PropertyNamespace & namespaceToAdd)
        {
            namespaces.push_back(namespaceToAdd);
            previousModification = Modification::NamespaceAdded;
        }

        void AddValue(std::string const & name, std::string const & value)
        {
            values[name] = value;
            lastValue.first = name;
            lastValue.second = value;
            previousModification = Modification::ValueAdded;
        }

        void AddValue(std::string const & name)
        {
            AddValue(name, "");
        }

        void SkipNamespace()
        {
            ++skippedNamespaceCount;
        }

        bool ValueExists(std::string const & name) const
        {
            return values.find(name) != values.end();
        }

        ValuePair & GetLastValuePair() { return lastValue; }
        picojson::value & GetJson() { return json; }

        size_t GetValueCount() const { return values.size(); }
        size_t GetNamespaceCount() const { return namespaces.size(); }
        size_t GetSkippedNamespaceCount() const { return skippedNamespaceCount; }
        Modification GetPreviousModification() const { return previousModification; }
        int GetDepth() const { return depth; }
        std::string const & GetName() const { return name; }
        std::string const & GetPath() const { return path; }
    private:
        Modification previousModification;
        std::map<std::string, std::string> values;
        ValuePair lastValue;
        std::vector<PropertyNamespace> namespaces;
        int depth;
        std::string name;
        std::string path;
        int skippedNamespaceCount;
        picojson::value & json;
    };

    class Rules
    {
    public:
        struct Conversion
        {
            enum Enum
            {
                None,
                Vector2,
                Vector4
            };
        };

        Rules(bool ignoreEmptyValues) : ignoreEmptyValues(ignoreEmptyValues)
        {
        }

        bool ShouldIgnoreEmptyValues() const
        {
            return ignoreEmptyValues;
        }

        bool ShouldInclude(std::string const & path) const
        {
            if (includes.empty())
            {
                return true;
            }

            bool shouldInclude = false;

            for (std::string const & include : includes)
            {
                if(IsMatch(path, include))
                {
                    shouldInclude = true;
                    break;
                }
            }

            return shouldInclude;
        }

        void ApplyValueReplacement(std::string const & path, std::string & valueOut) const
        {
            for (auto const & replacementPair : replacements)
            {
                if (IsMatch(path, replacementPair.first))
                {
                    std::regex fromExpr(replacementPair.second.first);
                    valueOut = std::regex_replace(valueOut, fromExpr, replacementPair.second.second);
                }
            }
        }

        bool ApplyConversion(PropertyNamespace & ns, PropertyNamespace & parentNs, std::ofstream & stream, Rules const & rules) const
        {
            bool consumeNamespace = false;

            for(auto & conversionPair :conversions)
            {
                if (IsMatch(ns.GetPath(), conversionPair.first))
                {
                    std::string newName = std::get<0>(conversionPair.second);

                    if(parentNs.ValueExists(newName))
                    {
                        newName += "_" + util::ToString(parentNs.GetValueCount());
                    }

                    consumeNamespace = std::get<1>(conversionPair.second);
                    Conversion::Enum const type = std::get<2>(conversionPair.second);
                    std::map<std::string, int> const & valueIndexMap = std::get<3>(conversionPair.second);
                    std::string newValue;

                    switch(type)
                    {
                    case Conversion::Vector2:
                        newValue = GetVectorValue(2, ns, valueIndexMap);
                        break;
                    case Conversion::Vector4:
                        newValue = GetVectorValue(4, ns, valueIndexMap);
                        break;
                    default:
                        assert(false);
                    }

                    if(!newValue.empty())
                    {
                        if(consumeNamespace)
                        {
                            parentNs.AddValue(newName, newValue);
                            WriteProperty(parentNs, newName, newValue, stream, rules);
                        }
                        else
                        {
                            picojson::object & nsObject = ns.GetJson().get<picojson::object>();

                            for(auto const & valueIndexPair : valueIndexMap)
                            {
                                auto itr = nsObject.find(valueIndexPair.first);

                                if(itr != nsObject.end())
                                {
                                    nsObject.erase(itr);
                                }
                            }

                            nsObject[newName] = picojson::value(newValue);
                        }
                    }
                    break;
                }
            }

            return consumeNamespace;
        }

        void AddInclude(std::string const & include)
        {
            if (std::find(includes.begin(), includes.end(), include) == includes.end())
            {
                includes.push_back(include);
            }
        }

        void AddVariable(std::string const & name, std::string const & value)
        {
            variables[name] = value;
        }

        void AddReplacement(std::string const & path, std::string const & from, std::string const & to)
        {
            replacements.push_back(std::make_pair(path, std::make_pair(from, to)));
        }

        void AddConversion(std::string const & path, std::string const & name, bool preserveNamespace, Conversion::Enum type, std::map<std::string, int> const & valueOrderMapping)
        {
            conversions.push_back(std::make_pair(path, std::make_tuple(name, preserveNamespace, type, valueOrderMapping)));
        }

        void ExpandVariables()
        {
            for (auto & varPair : variables)
            {
                bool requiresExpansion = true;

                while(requiresExpansion)
                {
                    requiresExpansion = false;

                    for (auto & varPairNested : variables)
                    {
                        util::ReplaceAll(varPair.second, varPairNested.first, varPairNested.second);
                        requiresExpansion |= varPair.second.find("{") != std::string::npos || varPair.second.find("}") != std::string::npos;
                    }
                }
            }

            for(auto & include : includes)
            {
                ExpandVariables(include);
            }

            for(auto & replacementPair : replacements)
            {
                ExpandVariables(replacementPair.first);
            }

            for(auto & conversionPair : conversions)
            {
                ExpandVariables(conversionPair.first);
            }
        }
    private:
        bool IsMatch(std::string const & path, std::string const & pattern) const
        {
            std::regex pathExpr(pattern);
            std::smatch match;
            return std::regex_search(path, match, pathExpr) && match.length() == path.length();
        }

        std::string GetVectorValue(int size, PropertyNamespace & ns, std::map<std::string, int> const & valueIndexMap) const
        {
            assert(valueIndexMap.size() == size);
            assert(ns.GetJson().is<picojson::object>());

            picojson::object & nsObject = ns.GetJson().get<picojson::object>();
            std::vector<std::string> values;
            values.resize(size);

            for(auto const & valueIndexPair : valueIndexMap)
            {
                values[valueIndexPair.second] = valueIndexPair.first;
            }

            std::string vectorValue;
            for(std::string const & value : values)
            {
                if(!vectorValue.empty())
                {
                    vectorValue += ", ";
                }

                vectorValue += nsObject[value].to_str();
            }

            return vectorValue;
        }

        void ExpandVariables(std::string & input)
        {
            for (auto & varPair : variables)
            {
                util::ReplaceAll(input, varPair.first, varPair.second);
            }
        }

        bool ignoreEmptyValues;
        std::vector<std::string> includes;
        std::map<std::string, std::string> variables;
        std::vector<std::pair<std::string, std::pair<std::string, std::string>>> replacements;
        std::vector<std::pair<std::string, std::tuple<std::string, bool, Conversion::Enum, std::map<std::string, int>>>> conversions;
    };

    bool IsNamespaceType(picojson::value const & node)
    {
        // Treat arrays and objects as namespaces since the property format has no equivalent concepts
        return node.is<picojson::object>() || node.is<picojson::array>();
    }

    std::string GetIndentation(int depth)
    {
        static const int indentationSpaces = 2;
        return std::string(depth * indentationSpaces, ' ');
    }

    bool AddNamespaceValue(PropertyNamespace & ns, std::string const & name, Rules const & rules)
    {
        if (rules.ShouldInclude(ns.GetPath() + "/" + name))
        {
            ns.AddValue(name);
            return true;
        }

        return false;
    }

    bool BeginWriteNamespace(PropertyNamespace & newNamespace, PropertyNamespace & parent, std::ofstream & stream, Rules const & rules)
    {
        bool skip = !rules.ShouldInclude(newNamespace.GetPath());

        if(!skip)
        {
            skip = rules.ApplyConversion(newNamespace, parent, stream, rules);
        }

        if(skip)
        {
            parent.SkipNamespace();
            return false;
        }

        // Add vertical spacing between namespaces and values at the same depth
        if (parent.GetPreviousModification() != PropertyNamespace::Modification::None)
        {
            if(parent.GetPreviousModification() == PropertyNamespace::Modification::ValueAdded)
            {
                stream << std::endl;
            }

            stream << std::endl;
        }

        std::string const indentation = GetIndentation(newNamespace.GetDepth());
        stream << indentation << newNamespace.GetName() << std::endl;
        stream << indentation << "{" << std::endl;
        return true;
    }

    void EndWriteNamespace(PropertyNamespace & newNamespace, std::ofstream & stream, Rules const & rules)
    {
        assert(rules.ShouldInclude(newNamespace.GetPath()));

        if (newNamespace.GetPreviousModification() == PropertyNamespace::Modification::ValueAdded)
        {
            stream << std::endl;
        }

        stream << GetIndentation(newNamespace.GetDepth()) << "}" << std::endl;
    }

    void WriteProperty(PropertyNamespace & currentNamespace, std::string const & name, std::string & value, std::ofstream & stream, Rules const & rules)
    {
        if(value.empty() && rules.ShouldIgnoreEmptyValues())
        {
            return;
        }

        std::string const path = currentNamespace.GetPath() + "/" + name;
        rules.ApplyValueReplacement(path, value);

        // Add vertical spacing between namespaces and values at the same depth but allow sequential value
        // declarations to group together
        if (currentNamespace.GetValueCount() > 1 && currentNamespace.GetPreviousModification() != PropertyNamespace::Modification::NamespaceAdded)
        {
            stream << std::endl;
        }

        stream << GetIndentation(currentNamespace.GetDepth() + 1) << name << " = " << value;
    }

    void ConvertAndExport(picojson::value & currentNode, converter::PropertyNamespace & currentNamespace, std::ofstream & stream, Rules const & rules)
    {
        if (currentNode.is<picojson::array>())
        {
            int valueIndex = 0;

            for (auto & arrayValue : currentNode.get<picojson::array>())
            {
                if (IsNamespaceType(arrayValue))
                {
                    // Create a nested namespace
                    std::string keyName;
                    arrayValue.get(keyName);
                    PropertyNamespace newNamespace(arrayValue, keyName, currentNamespace.GetDepth() + 1, currentNamespace);
                    if(BeginWriteNamespace(newNamespace, currentNamespace, stream, rules))
                    {
                        ConvertAndExport(arrayValue, newNamespace, stream, rules);
                        currentNamespace.AddNamespace(newNamespace);
                        EndWriteNamespace(newNamespace, stream, rules);
                    }
                }
                else
                {
                    // Array values are converted into key/value pairs within this namespace where
                    // they key is the values index into the array
                    if(AddNamespaceValue(currentNamespace, util::ToString(valueIndex), rules))
                    {
                        ConvertAndExport(arrayValue, currentNamespace, stream, rules);
                        ++valueIndex;
                    }
                }
            }
        }
        else if (currentNode.is<picojson::object>())
        {
            // Same as arrays, creates nested namespaces and pairs, the pairs aren't indexed since
            // they already have names

            for (auto & valuePair : currentNode.get<picojson::object>())
            {
                bool canExport = false;
                PropertyNamespace * nextNamespace = &currentNamespace;
                PropertyNamespace * newNamespace = nullptr;

                if (IsNamespaceType(valuePair.second))
                {
                    newNamespace = new PropertyNamespace(valuePair.second, valuePair.first, currentNamespace.GetDepth() + 1, currentNamespace);

                    if(BeginWriteNamespace(*newNamespace, currentNamespace, stream, rules))
                    {
                        nextNamespace = newNamespace;
                        canExport = true;
                    }
                    else
                    {
                        delete newNamespace;
                    }
                }
                else
                {
                    if(AddNamespaceValue(*nextNamespace, valuePair.first, rules))
                    {
                        canExport = true;
                    }
                }

                if(canExport)
                {
                    ConvertAndExport(valuePair.second, *nextNamespace, stream, rules);
                }

                if (nextNamespace == newNamespace)
                {
                    currentNamespace.AddNamespace(*newNamespace);
                    EndWriteNamespace(*newNamespace, stream, rules);
                    delete newNamespace;
                }
            }
        }
        else
        {
            // The entry for a value already exists in the current namespace and will always be the most recently
            // added so just fill in the value
            ValuePair & valuePair = currentNamespace.GetLastValuePair();
            valuePair.second = currentNode.to_str();
            WriteProperty(currentNamespace, valuePair.first, valuePair.second, stream, rules);
        }
    }
}

int main(int argc, char** argv)
{
    try
    {
        TCLAP::CmdLine cmd("JSON to Gameplay3D property converter");
        TCLAP::ValueArg<std::string> inputStreamArg("i", "input", "The JSON file to convert", true, "", "filepath", nullptr);
        TCLAP::ValueArg<std::string> outputFileArg("o", "output", "The Gameplay3D property file to output", true, "", "filepath", nullptr);
        TCLAP::ValueArg<std::string> rulesStreamArg("r", "rules", "[optional] A JSON file defining rules to apply to the output", false, "", "filepath", nullptr);
        TCLAP::ValueArg<bool> ignoreEmptyArg("e", "ignore-empty", "[optional] Ignore empty values, true by default", false, true, "true of false", nullptr);
        cmd.add(inputStreamArg);
        cmd.add(outputFileArg);
        cmd.add(rulesStreamArg);
        cmd.add(ignoreEmptyArg);
        cmd.parse(argc, argv);

        std::ifstream inputStream;
        inputStream.open(inputStreamArg.getValue(), std::ios::in);
        std::string errorMessage;

        if (inputStream)
        {
            picojson::value jsonDoc;
            std::cout << "Parsing JSON..." << std::endl;
            errorMessage = picojson::parse(jsonDoc, inputStream);

            if (errorMessage.empty())
            {
                converter::Rules rules(ignoreEmptyArg.getValue());

                if (!rulesStreamArg.getValue().empty())
                {
                    inputStream.close();
                    std::cout << "Parsing rules JSON..." << std::endl;
                    inputStream.open(rulesStreamArg.getValue(), std::ios::in);
                    picojson::value jsonRulesDoc;
                    errorMessage = picojson::parse(jsonRulesDoc, inputStream);

                    if (errorMessage.empty())
                    {
                        picojson::object const & root = jsonRulesDoc.get<picojson::object>();

                        auto varsItr = root.find("vars");
                        if (varsItr != root.end())
                        {
                            picojson::array const & vars = varsItr->second.get<picojson::array>();

                            for (auto & varObject : vars)
                            {
                                picojson::object const & varMap = varObject.get<picojson::object>();

                                for (auto & varPair : varMap)
                                {
                                    rules.AddVariable("{" + varPair.first + "}", varPair.second.to_str());
                                }
                            }
                        }

                        auto includesItr = root.find("includes");
                        if (includesItr != root.end())
                        {
                            picojson::array const & includes = includesItr->second.get<picojson::array>();

                            for (auto & includeObject : includes)
                            {
                                std::string includePath = includeObject.to_str();
                                rules.AddInclude(includePath);
                            }
                        }

                        auto replacementsItr = root.find("replacements");
                        if (replacementsItr != root.end())
                        {
                            picojson::array const & replacements = replacementsItr->second.get<picojson::array>();

                            for (auto & replacementPairObject : replacements)
                            {
                                auto & pathPair = *replacementPairObject.get<picojson::object>().begin();
                                auto & valuePair = *pathPair.second.get<picojson::object>().begin();
                                rules.AddReplacement(pathPair.first, valuePair.first, valuePair.second.to_str());
                            }
                        }

                        auto conversionsItr = root.find("conversions");
                        if (conversionsItr != root.end())
                        {
                            picojson::array const & conversions = conversionsItr->second.get<picojson::array>();

                            for (auto & conversionsPairObject : conversions)
                            {
                                auto & pathPair = *conversionsPairObject.get<picojson::object>().begin();
                                auto & namePair = *pathPair.second.get<picojson::object>().begin();
                                auto & typePair = *namePair.second.get<picojson::object>().begin();
                                auto & optionsPair = *typePair.second.get<picojson::object>().begin();

                                bool const preserveNamespace = optionsPair.first == "true";
                                converter::Rules::Conversion::Enum conversionType = converter::Rules::Conversion::None;

                                picojson::array const & mappingArray = optionsPair.second.get<picojson::array>();
                                std::map<std::string, int> mapping;
                                int mappingIndex = 0;
                                for(auto & value : mappingArray)
                                {
                                    mapping[value.to_str()] = mappingIndex;
                                    ++mappingIndex;
                                }
                                if(typePair.first == "Vector2")
                                {
                                    conversionType = converter::Rules::Conversion::Vector2;
                                }
                                else if(typePair.first == "Vector4")
                                {
                                    conversionType = converter::Rules::Conversion::Vector4;
                                }
                                else
                                {
                                    assert(false);
                                }

                                if(conversionType != converter::Rules::Conversion::None)
                                {
                                    rules.AddConversion(pathPair.first, namePair.first, preserveNamespace, conversionType, mapping);
                                }
                            }
                        }

                        rules.ExpandVariables();
                    }
                }

                if (errorMessage.empty())
                {
                    std::ofstream outputStream(outputFileArg.getValue());

                    if (outputStream)
                    {
                        std::cout << "Converting..." << std::endl;
                        converter::PropertyNamespace rootNamespace(jsonDoc);
                        converter::ConvertAndExport(jsonDoc, rootNamespace, outputStream, rules);
                        std::cout << "Done" << std::endl;
                    }
                    else
                    {
                        errorMessage = "Failed to create output stream: " + errorMessage;
                    }
                }
                else
                {
                    errorMessage = "Failed to open rules file: " + errorMessage;
                }
            }
        }
        else
        {
            errorMessage = "Failed to open input file";
        }

        if (!errorMessage.empty())
        {
            std::cerr << errorMessage.c_str() << std::endl;
        }
    }
    catch (TCLAP::ArgException & commandLineException)
    {
        std::cerr << commandLineException.error() << " for arg " << commandLineException.argId() << std::endl;
    }
}
