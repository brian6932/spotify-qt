#include "spotify/spotify.hpp"

spt::Spotify::Spotify(lib::settings &settings,
	const lib::http_client &httpClient, QObject *parent)
	: QObject(parent), lib::spt::api(settings, httpClient)
{
}

void spt::Spotify::select_device(const std::vector<lib::spt::device> &devices,
	lib::callback<lib::spt::device> &callback)
{
	auto *parentWidget = dynamic_cast<QWidget *>(parent());
	auto *dialog = new Dialog::DeviceSelect(devices, parentWidget);
	QDialog::connect(dialog, &Dialog::DeviceSelect::deviceSelected, callback);
	dialog->open();
}

auto spt::Spotify::tryRefresh() -> bool
{
	auto *parentWidget = dynamic_cast<QWidget *>(parent());

	try
	{
		refresh();
	}
	catch (const nlohmann::json::exception &e)
	{
		QMessageBox::warning(parentWidget, "Connection failed",
			QString("Failed to parse response from Spotify:\n%1").arg(e.what()));
		return false;
	}
	catch (const lib::spt::error &e)
	{
		QMessageBox::warning(parentWidget, "Connection failed",
			QString("Unexpected response:\n%1").arg(e.what()));
		return false;
	}
	catch (const std::exception &e)
	{
		QMessageBox::warning(parentWidget, "Connection failed",
			QString("Failed to connect to Spotify, check your connection and try again:\n%1")
				.arg(e.what()));
		return false;
	}

	return true;
}
