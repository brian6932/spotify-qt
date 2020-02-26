#include "settings.hpp"

Settings::Settings()
{
	settings = new QSettings();
}

Settings::~Settings()
{
	delete settings;
}

QString Settings::accessToken()
{
	return settings->value("AccessToken").toString();
}

void Settings::setAccessToken(QString &value)
{
	settings->setValue("AccessToken", value);
}

QString Settings::refreshToken()
{
	return settings->value("RefreshToken").toString();
}

void Settings::setRefreshToken(QString &value)
{
	settings->setValue("RefreshToken", value);
}

QString Settings::clientId()
{
	return settings->value("ClientId").toString();
}

void Settings::setClientId(const QString &value)
{
	settings->setValue("ClientId", value);
}

QString Settings::clientSecret()
{
	return settings->value("ClientSecret").toString();
}

void Settings::setClientSecret(const QString &value)
{
	settings->setValue("ClientSecret", value);
}

QString Settings::style()
{
	return settings->value("Style").toString();
}

void Settings::setStyle(const QString &value)
{
	settings->setValue("Style", value);
}

QString Settings::sptPath()
{
	return settings->value("Spotify/Path").toString();
}

void Settings::setSptPath(const QString &value)
{
	settings->setValue("Spotify/Path", value);
}

bool Settings::sptStartClient()
{
	return settings->value("Spotify/StartClient").toBool();
}

void Settings::setSptStartClient(bool value)
{
	settings->setValue("Spotify/StartClient", value);
}

QString Settings::sptUser()
{
	return settings->value("Spotify/Username").toString();
}

void Settings::setSptUser(const QString &value)
{
	settings->setValue("Spotify/Username", value);
}

int Settings::sptBitrate()
{
	return settings->value("Spotify/Bitrate").toInt();
}

void Settings::setSptBitrate(int value)
{
	settings->setValue("Spotify/Bitrate", value);
}

bool Settings::pulseVolume()
{
	return settings->value("PulseVolume").toBool();
}

void Settings::setPulseVolume(bool value)
{
	settings->setValue("PulseVolume", value);
}

QString Settings::lastPlaylist()
{
	return settings->value("LastPlaylist").toString();
}

void Settings::setLastPlaylist(const QString &value)
{
	settings->setValue("LastPlaylist", value);
}