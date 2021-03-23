#include "lib/cache.hpp"

lib::cache::cache(const lib::paths &paths)
	: paths(paths)
{
}

ghc::filesystem::path lib::cache::dir(const std::string &type)
{
	auto file_dir = ghc::filesystem::path(paths.cache()) / type;

	if (!ghc::filesystem::exists(file_dir))
		ghc::filesystem::create_directories(file_dir);

	return file_dir;
}

std::string lib::cache::file(const std::string &id)
{
	return fmt::format("{}.json", id);
}

ghc::filesystem::path lib::cache::path(const std::string &type, const std::string &id)
{
	return dir(type) / file(id);
}

//region album

//endregion

//region playlist

lib::spt::playlist lib::cache::get_playlist(const std::string &id)
{
	try
	{
		return json::load<lib::spt::playlist>(path("playlist", id));
	}
	catch (const std::exception &e)
	{
		log::warn("Failed to load playlist from cache: {}", e.what());
	}

	return lib::spt::playlist();
}

void lib::cache::set_playlist(const spt::playlist &playlist)
{
	lib::json::save(path("playlist", playlist.id), playlist);
}

//endregion

//region tracks

std::vector<lib::spt::track> lib::cache::tracks(const std::string &id)
{
	auto file_path = path("tracks", id);

	if (!ghc::filesystem::exists(file_path))
		return std::vector<lib::spt::track>();

	return lib::json::load_items<lib::spt::track>(file_path);
}

void lib::cache::tracks(const std::string &id, const std::vector<lib::spt::track> &tracks)
{
	lib::json::save_items(path("tracks", id), tracks);
}

std::map<std::string, std::vector<lib::spt::track>> lib::cache::all_tracks()
{
	auto dir = ghc::filesystem::path(paths.cache()) / "tracks";
	std::map<std::string, std::vector<lib::spt::track>> results;

	if (!ghc::filesystem::exists(dir))
		return results;

	for (const auto &entry : ghc::filesystem::directory_iterator(dir))
	{
		auto id = entry.path().filename().replace_extension().string();
		results[id] = tracks(id);
	}

	return results;
}

//endregion
