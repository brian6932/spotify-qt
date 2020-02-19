#pragma once

#include "src/settings.hpp"
#include "playlist.hpp"
#include "device.hpp"
#include "playback.hpp"

#include <QDateTime>
#include <QtNetwork>
#include <QSettings>
#include <QProcessEnvironment>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QDesktopServices>
#include <QInputDialog>
#include <QCoreApplication>

namespace spt
{
	class Spotify : QObject
	{
	public:
		Spotify();
		~Spotify() override;
		/**
		 * Authenticate with Spotify
		 */
		bool auth();
		/**
		 * HTTP GET request expecting JSON response
		 */
		QJsonDocument get(QString url);
		void playlists(QVector<Playlist> *playlists);
		QVector<Device> devices();
		bool setDevice(Device device);
		bool playTracks(QStringList &trackIds);
		bool setShuffle(bool enabled);
		Playback currentPlayback();
		bool pause();
		bool resume();
	private:
		QDateTime *lastAuth;
		QNetworkAccessManager *networkManager;
		/**
		 * Prepare network request with auth header
		 */
		QNetworkRequest request(QString &url);
		/**
		 * HTTP PUT request expecting JSON response
		 */
		void put(QString url, QVariantMap *body = nullptr);

		bool refresh();

		static QString clientId();
		static QString clientSecret();
	};
}