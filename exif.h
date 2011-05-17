/*
 * Wally - Qt4 wallpaper/background changer
 * Copyright (C) 2009  Antonio Di Monaco <tony@becrux.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef EXIF_H
#define EXIF_H

#include <QtCore>
//#include <QtGui>

#ifdef EXIF
  #include <libexif/exif-byte-order.h>
  #include <libexif/exif-ifd.h>
  #include <libexif/exif-content.h>
  #include <libexif/exif-entry.h>
  #include <libexif/exif-data.h>
  #include <libexif/exif-tag.h>

  Q_DECLARE_METATYPE(ExifRational)
  Q_DECLARE_METATYPE(ExifSRational)
#endif

// #include "gui.h"

namespace Exif
{
  struct Tag
  {
    QString title;
    QString description;

    Tag() { }
    Tag(const Tag &tag) : title(tag.title), description(tag.description) { }
    Tag(const QString &t, const QString &d) : title(t), description(d) { }
  };

  struct Data
  {
    QVariant value;
    QString readableValue;

    Data() { }
    Data(const Data &data) : value(data.value), readableValue(data.readableValue) { }
    Data(const QVariant &v, const QString &rV) : value(v), readableValue(rV) { }
  };

  class Tags : public QMap<int, QPair<Tag, Data> >
  {
    QByteArray _data;
    QString _fileName;

  #ifdef EXIF
    static ExifByteOrder byteOrder;
    static void readEntry(ExifEntry *entry, void *data);
    static void readContent(ExifContent *content, void *data);
  #endif

    void readExifData(const QByteArray &data);
  
  public:
    Tags(const QFileInfo &fileInfo);
    Tags(const QByteArray &data);

    QString fileName() const { return _fileName; }

  #ifdef EXIF
    QString owner() const
      { return (value(EXIF_TAG_XP_AUTHOR).second.readableValue.isEmpty())?
                value(EXIF_TAG_ARTIST).second.readableValue :
                value(EXIF_TAG_XP_AUTHOR).second.readableValue; }
    QString title() const
      { return (value(EXIF_TAG_XP_AUTHOR).second.readableValue.isEmpty())?
                value(EXIF_TAG_IMAGE_DESCRIPTION).second.readableValue :
                value(EXIF_TAG_XP_AUTHOR).second.readableValue; }
    QString description() const
      { return (value(EXIF_TAG_XP_COMMENT).second.readableValue.isEmpty())?
                value(EXIF_TAG_USER_COMMENT).second.readableValue :
                value(EXIF_TAG_XP_COMMENT).second.readableValue; }
  #else
    QString owner() const { return QString(); }
    QString title() const { return QString(); }
    QString description() const { return QString(); }
  #endif

    QByteArray data() const { return _data; }

    QImage normalize(const QImage &image);
  };
/*
  class InfoDialog : public Gui::Dialog
  {
    Q_OBJECT

    void setupUi(const Tags &tags);

  public:
    InfoDialog(const QFileInfo &fileInfo, QWidget *parent = 0);
    InfoDialog(const QByteArray &data, QWidget *parent = 0);
    virtual ~InfoDialog() { }
  };
*/
}

#endif
