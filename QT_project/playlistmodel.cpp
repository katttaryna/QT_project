#include "playlistmodel.h"

#include <QFileInfo>
#include <QUrl>
#include <QMediaPlaylist>

PlaylistModel::PlaylistModel(QObject *parent)
    :QAbstractItemModel(parent){

}

PlaylistModel::~PlaylistModel(){

}

int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    return m_playlist && !parent.isValid() ? m_playlist-> mediaCount() : 0;
}

int PlaylistModel::columnCount(const QModelIndex &parent) const
{
    return !parent.isValid() ? ColumnCount : 0;
}

QModelIndex PlaylistModel::index(int row, int column, const QModelIndex &parent) const
{
    return m_playlist && !parent.isValid()
                && row >= 0 && row < m_playlist->mediaCount()
                && column >= 0 && column < ColumnCount
            ? createIndex(row, column)
            : QModelIndex();
}


// через Q_UNUSED (предотвращает чрезмерную реакцию компилятора на более чем один случай не обработанного параметра ; м создать экземпляр объекта без единого вызова от него)
QModelIndex PlaylistModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}


// isValid действителен ли адрес
//если вернет false -> очистить адрес или начать все сначала
QVariant PlaylistModel::data(const QModelIndex &index , int role ) const
{
    if (index.isValid() && role == Qt::DisplayRole)
    {
        QVariant value = m_data[index];
        if (!value.isValid() && index.column () == Title ){
            QUrl location = m_playlist->media(index.row()).canonicalUrl();
            return QFileInfo(location.path()).fileName();
        }
        return value;
    }
    return QVariant();
}

QMediaPlaylist *PlaylistModel::playlist() const
{
    return m_playlist.data();
}

void PlaylistModel::setPlaylist(QMediaPlaylist *playlist)

{
    if(m_playlist)
    {
        disconnect(m_playlist.data(), &QMediaPlaylist::mediaAboutToBeInserted,this, &PlaylistModel::beginInsertItems);//insertItems вставка
        disconnect(m_playlist.data(), &QMediaPlaylist::mediaInserted, this, &PlaylistModel::endInsertItems); //mediaInsert вставить в плейлист в позицию pos
        disconnect(m_playlist.data(), &QMediaPlaylist::mediaAboutToBeRemoved, this, &PlaylistModel::beginRemoveItems);
        disconnect(m_playlist.data(),&QMediaPlaylist::mediaRemoved, this, &PlaylistModel::endRemoveItems);
        disconnect(m_playlist.data(), &QMediaPlaylist::mediaChanged, this,&PlaylistModel::changeItems);

    }

    beginResetModel();
    m_playlist.reset(playlist);
            if(m_playlist)
            {
                connect(m_playlist.data(), &QMediaPlaylist::mediaAboutToBeInserted, this,&PlaylistModel::beginInsertItems);
                connect(m_playlist.data(), &QMediaPlaylist::mediaInserted, this, &PlaylistModel::endInsertItems);
                connect(m_playlist.data(), &QMediaPlaylist::mediaAboutToBeRemoved, this, &PlaylistModel::beginRemoveItems);
                connect(m_playlist.data(), &QMediaPlaylist::mediaRemoved, this, &PlaylistModel::endRemoveItems);
                connect(m_playlist.data(), &QMediaPlaylist::mediaChanged ,this, &PlaylistModel::changeItems);
            }

            endResetModel();
}


// emit вызов всех слотов
bool PlaylistModel::setData(const QModelIndex & index, const QVariant &value, int role)
{
    Q_UNUSED(role);
    m_data[index] = value;
    emit dataChanged(index, index);
            return true;
}

void PlaylistModel::beginInsertItems(int start, int end)
{
    m_data.clear();
    beginInsertRows(QModelIndex(), start, end);
}

void PlaylistModel::endInsertItems()
{
    endInsertRows();
}

void PlaylistModel::beginRemoveItems(int start, int end)
{
    m_data.clear();
    beginRemoveRows(QModelIndex(), start, end);
}

void PlaylistModel::endRemoveItems()
{
    endInsertRows();
}

void PlaylistModel::changeItems(int start, int end)
{
    m_data.clear();
    emit dataChanged(index(start,0), index(end, ColumnCount));
}



//insertRows вставка строки (выз при внедр в ф )










