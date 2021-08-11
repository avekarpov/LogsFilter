#include <iostream>
#include <filesystem>
#include <list>
#include <fstream>
#include <string>

#include "ThreadPool.h"

bool filterLogs(const std::filesystem::path &from, const std::filesystem::path &to, const std::vector<std::string> &filters, bool inversionFilters = false)
{
    static const unsigned INDENT = 22;
    
    std::ifstream fin(from);
    std::ofstream fout(to);
    
    if(!fin.is_open() || !fout.is_open())
    {
        return false;
    }
    
    std::string line;
    
    bool isSomeFilterFound;
    
    while(std::getline(fin, line))
    {
        isSomeFilterFound = false;
        
        for(const auto &filter : filters)
        {
            if(INDENT + filter.size() <= line.size() && line.substr(INDENT, filter.size()) == filter)
            {
                isSomeFilterFound = true;
                
                if(!inversionFilters)
                {
                    fout << line << std::endl;
                }
                
                break;
            }
        }
        if(inversionFilters && !isSomeFilterFound)
        {
            fout << line << std::endl;
        }
    }
    
    fin.close();
    fout.close();
    
    return true;
}

auto getListLogs(const std::filesystem::path &fromDir, const std::string &extension = ".log")
{
    std::list<std::filesystem::path> listLogs;
    
    for(auto &log : std::filesystem::recursive_directory_iterator(fromDir))
    {
        const auto &path = log.path();
        
        if(!log.is_directory() && path.has_extension() && path.extension() == extension)
        {
            listLogs.push_back(path);
        }
    }
    
    return listLogs;
}

int main(int argc, char *argv[])
{
    ThreadPool threadPool;
    std::vector<std::future<bool>> results;
    
    std::filesystem::path fromDir;
    std::filesystem::path toDir;
    std::vector<std::string> filters;
    bool inversionFilters = false;
    
    std::list<std::filesystem::path> listLogs;
    std::list<std::filesystem::path> listFilteredLogs;
    
    fromDir = argv[1];
    toDir = argv[2];
    
    filters.reserve(argc - 4);
    for(int i = 3; i < argc - 1; ++i)
    {
        filters.emplace_back(argv[i]);
    }
    
    if(std::strcmp(argv[argc - 1], "-i") == 0)
    {
        inversionFilters = true;
    }
    else
    {
        filters.emplace_back(argv[argc - 1]);
    }
    
    listLogs = std::move(getListLogs(fromDir));
    
    for(const auto &log : listLogs)
    {
        std::filesystem::path filteredLog = toDir.string() + log.string().substr(fromDir.string().size(), log.string().size() - fromDir.string().size());
        
        if(!std::filesystem::exists(filteredLog.parent_path()))
        {
            std::filesystem::create_directories(filteredLog.parent_path());
        }
        
        results.push_back(threadPool.enqueue(filterLogs, std::ref(log), filteredLog, std::ref(filters), inversionFilters));
    }
    
    {
        auto listLogsIt = listLogs.begin();
        for(auto &result : results)
        {
            std::cout << listLogsIt->string().substr(fromDir.string().size(), listLogsIt->string().size() - fromDir.string().size()) << " = ";
            if(result.get())
            {
                std::cout << "Okay";
            }
            else
            {
                std::cout << "False";
            }
            std::cout << std::endl;
            
            ++listLogsIt;
        }
    }
    
    return 0;
}