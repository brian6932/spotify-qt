#include "leftsidepanel.hpp"

#include "mainwindow.hpp"

LeftSidePanel::LeftSidePanel(spt::Spotify &spotify, lib::settings &settings,
	spt::Current &current, QWidget *parent)
	: spotify(spotify),
	settings(settings),
	current(current),
	QWidget(parent)
{
	auto layout = new QVBoxLayout(this);
	setMaximumWidth(250); // TODO: ?

	// Library
	libraryList = new LibraryList(spotify, parent);
	auto library = Utils::createGroupBox(QVector<QWidget *>() << libraryList, parent);
	library->setTitle("Library");
	layout->addWidget(library);

	// Playlists
	playlists = new PlaylistList(spotify, parent);
	refreshPlaylists();
	auto playlistContainer = Utils::createGroupBox(QVector<QWidget *>() << playlists, parent);
	playlistContainer->setTitle("Playlists");
	layout->addWidget(playlistContainer);

	// Current context info
	auto contextLayout = new QHBoxLayout();
	contextIcon = new QLabel(this);
	contextIcon->setVisible(false);
	contextInfo = new QLabel(this);
	contextInfo->setToolTip("Currently playing from");
	contextInfo->setVisible(false);
	contextLayout->addSpacing(16);
	contextLayout->addWidget(contextIcon);
	contextLayout->addWidget(contextInfo, 1);
	layout->addLayout(contextLayout);
	contextInfo->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	QWidget::connect(contextInfo, &QWidget::customContextMenuRequested,
		this, &LeftSidePanel::contextInfoMenu);

	// Now playing song
	auto nowPlayingLayout = new QHBoxLayout();
	nowPlayingLayout->setSpacing(12);
	nowAlbum = new QLabel(this);
	nowAlbum->setFixedSize(64, 64);
	nowAlbum->setPixmap(Icon::get("media-optical-audio").pixmap(nowAlbum->size()));
	nowPlayingLayout->addWidget(nowAlbum);
	nowPlaying = new QLabel("No music playing", this);
	nowPlaying->setWordWrap(true);
	nowPlayingLayout->addWidget(nowPlaying);
	layout->addLayout(nowPlayingLayout);

	// Show menu when clicking now playing
	nowPlaying->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	QLabel::connect(nowPlaying, &QWidget::customContextMenuRequested,
		this, &LeftSidePanel::popupSongMenu);
}

void LeftSidePanel::popupSongMenu(const QPoint &pos)
{
	auto track = current.playback.item;
	if (track.name.empty() && track.artist.empty())
		return;
	(new SongMenu(track, spotify, parentWidget()))->popup(nowPlaying->mapToGlobal(pos));
}

int LeftSidePanel::latestTrack(const std::vector<lib::spt::track> &tracks)
{
	auto latest = 0;
	for (int i = 0; i < tracks.size(); i++)
	{
		if (DateUtils::fromIso(tracks[i].added_at)
			> DateUtils::fromIso(tracks[latest].added_at))
		{
			latest = i;
		}
	}
	return latest;
}

QIcon LeftSidePanel::currentContextIcon() const
{
	return Icon::get(QString("view-media-%1")
		.arg(current.playback.context.type.empty()
			? "track"
			: current.playback.context.type == "album"
				? "album-cover"
				: QString::fromStdString(current.playback.context.type)));
}

void LeftSidePanel::updateContextIcon()
{
	if (!settings.general.show_context_info)
	{
		contextIcon->setVisible(false);
		contextInfo->setVisible(false);
		return;
	}

	auto currentName = current.playback.context.type.empty()
		|| current.playback.context.uri.empty()
		? "No context"
		: current.playback.context.type == "album"
			? current.playback.item.album
			: current.playback.context.type == "artist"
				? current.playback.item.artist
				: getPlaylistName(current.playback.context.uri);

	contextInfo->setText(QString::fromStdString(currentName));
	auto size = contextInfo->fontInfo().pixelSize();
	contextIcon->setPixmap(currentContextIcon().pixmap(size, size));

	auto show = currentName != "No context";
	contextIcon->setVisible(show);
	contextInfo->setVisible(show);
}

void LeftSidePanel::contextInfoMenu(const QPoint &pos)
{
	auto menu = new QMenu(contextInfo);

	if (lib::developer_mode::enabled)
	{
		auto devContext = menu->addAction(current.context);
		devContext->setEnabled(false);
	}

	auto open = menu->addAction(currentContextIcon(), QString("Open %1")
		.arg(QString::fromStdString(current.playback.context.type)));
	QAction::connect(open, &QAction::triggered, this, &LeftSidePanel::contextInfoOpen);

	menu->popup(contextInfo->mapToGlobal(pos));
}

void LeftSidePanel::contextInfoOpen(bool)
{
	auto mainWindow = MainWindow::find(parentWidget());
	auto type = current.playback.context.type;
	auto uri = lib::strings::split(current.playback.context.uri, ':').back();

	if (type == "album")
		mainWindow->loadAlbum(uri);
	else if (type == "artist")
		mainWindow->openArtist(uri);
	else if (type == "playlist")
	{
		auto playlist = spotify.playlist(uri);
		libraryList->setCurrentItem(nullptr);
		playlists->setCurrentRow(-1);
		mainWindow->loadPlaylist(playlist);
	}
}

std::unordered_set<std::string> LeftSidePanel::allArtists()
{
	std::unordered_set<std::string> artists;
	for (auto i = 0; i < playlists->count(); i++)
	{
		auto mainWindow = MainWindow::find(parentWidget());
		auto playlistId = playlists->item(i)->data(RolePlaylistId).toString().toStdString();

		for (auto &track : mainWindow->playlistTracks(playlistId))
			artists.insert(track.artist);
	}
	return artists;
}

//region Playlists

QListWidgetItem *LeftSidePanel::playlistItem(int index) const
{
	return playlists->item(index);
}

int LeftSidePanel::playlistItemCount() const
{
	return playlists->count();
}

void LeftSidePanel::setCurrentPlaylistItem(int index) const
{
	playlists->setCurrentRow(index);
}

QListWidgetItem *LeftSidePanel::currentPlaylist()
{
	return playlists->currentItem();
}

void LeftSidePanel::refreshPlaylists()
{
	QListWidgetItem *currentItem = nullptr;
	QString lastItem;
	if (currentPlaylist() != nullptr)
		lastItem = currentPlaylist()->data(RolePlaylistId).toString();

	sptPlaylists = spotify.playlists();

	// Add all playlists
	playlists->clear();
	auto i = 0;
	QTextDocument doc;
	for (auto &playlist : sptPlaylists)
	{
		auto item = new QListWidgetItem(QString::fromStdString(playlist.name), playlists);

		doc.setHtml(QString::fromStdString(playlist.description));
		item->setToolTip(doc.toPlainText());

		item->setData(RolePlaylistId, QString::fromStdString(playlist.id));
		item->setData(RoleIndex, i++);

		if (playlist.id == lastItem.toStdString())
			currentItem = item;
	}

	// Sort
	if (settings.general.playlist_order != lib::playlist_order_default)
		orderPlaylists(settings.general.playlist_order);

	if (currentItem != nullptr)
		playlists->setCurrentItem(currentItem);
}

void LeftSidePanel::orderPlaylists(lib::playlist_order order)
{
	QList<QListWidgetItem *> items;
	items.reserve(playlists->count());

	auto i = 0;
	while (playlists->item(0) != nullptr)
		items.insert(i, playlists->takeItem(0));

	QMap<QString, int> customOrder;
	MainWindow *mainWindow;

	switch (order)
	{
		case lib::playlist_order_default:
			std::sort(items.begin(), items.end(), [](QListWidgetItem *i1, QListWidgetItem *i2)
			{
				return i1->data(RoleIndex).toInt() < i2->data(RoleIndex).toInt();
			});
			break;

		case lib::playlist_order_alphabetical:
			std::sort(items.begin(), items.end(), [](QListWidgetItem *i1, QListWidgetItem *i2)
			{
				return i1->text() < i2->text();
			});
			break;

		case lib::playlist_order_recent:
			// TODO: Currently sorts by when tracks where added, not when playlist was last played
			mainWindow = MainWindow::find(parentWidget());
			if (mainWindow == nullptr)
			{
				lib::log::error("Failed to order playlist: MainWindow not found");
				break;
			}

			std::sort(items.begin(), items.end(),
				[mainWindow](QListWidgetItem *i1, QListWidgetItem *i2)
				{
					auto id1 = i1->data(DataRole::RolePlaylistId).toString().toStdString();
					auto id2 = i2->data(DataRole::RolePlaylistId).toString().toStdString();

					auto t1 = mainWindow->playlistTracks(id1);
					auto t2 = mainWindow->playlistTracks(id2);

					return !t1.empty() && !t2.empty()
						? DateUtils::fromIso(t1.at(latestTrack(t1)).added_at)
							> DateUtils::fromIso(t2.at(latestTrack(t2)).added_at)
						: false;
				});
			break;

		case lib::playlist_order_custom:
			i = 0;
			for (auto &playlist : settings.general.custom_playlist_order)
				customOrder[QString::fromStdString(playlist)] = i++;
			std::sort(items.begin(), items.end(),
				[customOrder](QListWidgetItem *i1, QListWidgetItem *i2)
				{
					auto id1 = i1->data(DataRole::RolePlaylistId).toString();
					auto id2 = i2->data(DataRole::RolePlaylistId).toString();

					return customOrder.contains(id1) && customOrder.contains(id2)
						? customOrder[id1] < customOrder[id2]
						: false;
				});
			break;
	}

	for (auto item : items)
		playlists->addItem(item);
}

int LeftSidePanel::playlistCount() const
{
	return sptPlaylists.size();
}

lib::spt::playlist &LeftSidePanel::playlist(size_t index)
{
	return sptPlaylists[index];
}

std::string LeftSidePanel::getPlaylistNameFromSaved(const std::string &id)
{
	for (auto &playlist : sptPlaylists)
	{
		if (lib::strings::ends_with(id, playlist.id))
			return playlist.name;
	}
	return std::string();
}

QString LeftSidePanel::getCurrentlyPlaying()
{
	return nowPlaying->text();
}

void LeftSidePanel::setCurrentlyPlaying(const QString &value)
{
	nowPlaying->setText(value);
}

void LeftSidePanel::setAlbumImage(const QPixmap &pixmap)
{
	nowAlbum->setPixmap(pixmap);
}

std::string LeftSidePanel::getPlaylistName(const std::string &id)
{
	auto name = getPlaylistNameFromSaved(id);
	if (!name.empty())
		return name;
	return spotify.playlist(lib::strings::split(id, ':').back()).name;
}

QTreeWidgetItem *LeftSidePanel::getCurrentLibraryItem()
{
	return libraryList->currentItem();
}

void LeftSidePanel::setCurrentLibraryItem(QTreeWidgetItem *item)
{
	libraryList->setCurrentItem(item);
}

std::vector<lib::spt::playlist> &LeftSidePanel::getPlaylists()
{
	return sptPlaylists;
}

//endregion