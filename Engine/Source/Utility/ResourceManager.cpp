#include "ResourceManager.hpp"

#include "Editor/Log.hpp"

namespace helios::utility
{
	void ResourceManager::LocateAssetsDirectory()
    {
        auto currentDirectory = std::filesystem::current_path();

        while (!std::filesystem::exists(currentDirectory / "Assets"))
        {
            if (currentDirectory.has_parent_path())
            {
                currentDirectory = currentDirectory.parent_path();
            }
            else
            {
                ErrorMessage(L"Assets Directory not found!");
            }
        }

        auto assetsDirectory = currentDirectory / "Assets";

        if (!std::filesystem::is_directory(assetsDirectory))
        {
            ErrorMessage(L"Assets Directory that was located is not a directory!");
        }

        sAssetsDirectory = currentDirectory.wstring() + L"/";

        editor::LogMessage(L"Detected assets path directory : " + sAssetsDirectory, editor::LogMessageTypes::Info);
    }

    std::wstring ResourceManager::GetAssetPath(std::wstring_view relativePath)
    {
        return std::move(sAssetsDirectory + relativePath.data());
    }
} // namespace helios::utility