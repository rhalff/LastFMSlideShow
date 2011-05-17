/*
TRANSLATOR Exif::InfoDialog
*/

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

#include <string.h>
#include <QtDebug>

#ifdef EXIF
  #include <libexif/exif-byte-order.h>
  #include <libexif/exif-ifd.h>
  #include <libexif/exif-content.h>
  #include <libexif/exif-entry.h>
  #include <libexif/exif-data.h>
  #include <libexif/exif-tag.h>
#endif

#include "exif.h"

using namespace Exif;

#ifdef EXIF
ExifByteOrder Tags::byteOrder;

void Tags::readEntry(ExifEntry *entry, void *data)
{
  char *buf = new char[1024];
  QVariant value;
  ExifRational rational;
  ExifSRational sRational;
  QMap<int, QPair<Tag, Data> > *exifValues = reinterpret_cast<QMap<int, QPair<Tag, Data> > *> (data);
  QString readableValue;
  QString title = QTextCodec::codecForLocale()->toUnicode(exif_tag_get_title_in_ifd(entry->tag,exif_entry_get_ifd(entry)));
  QString description = QTextCodec::codecForLocale()->toUnicode(exif_tag_get_description_in_ifd(entry->tag,exif_entry_get_ifd(entry)));

  memset(buf,0,1024);

  switch (entry->format)
  {
    case EXIF_FORMAT_BYTE:
      exif_entry_get_value(entry,buf,1024);
      readableValue = buf;
      (*exifValues)[entry->tag] = QPair<Tag,Data>(Tag(title,description),
                                                  Data(static_cast<unsigned int> (*entry->data),readableValue));
      break;

    case EXIF_FORMAT_ASCII:
      switch (entry->tag)
      {
        case EXIF_TAG_XP_TITLE:
        case EXIF_TAG_XP_COMMENT:
        case EXIF_TAG_XP_AUTHOR:
        case EXIF_TAG_XP_KEYWORDS:
        case EXIF_TAG_XP_SUBJECT:
          exif_entry_get_value(entry,buf,1024);
          readableValue = QString::fromUtf8(buf,1024);
          (*exifValues)[entry->tag] = QPair<Tag,Data>(Tag(title,description),
                                        Data(QString::fromUtf16(reinterpret_cast<const ushort *> (entry->data),entry->size).trimmed(),
                                             readableValue));
          break;

        default:
          exif_entry_get_value(entry,buf,1024);
          readableValue = buf;
          (*exifValues)[entry->tag] = QPair<Tag,Data>(Tag(title,description),
                                        Data(QString::fromLatin1(reinterpret_cast<const char *> (entry->data),entry->size).trimmed(),
                                             readableValue));
          break;
      }
      break;

    case EXIF_FORMAT_SHORT:
      exif_entry_get_value(entry,buf,1024);
      readableValue = buf;
      (*exifValues)[entry->tag] = QPair<Tag,Data>(Tag(title,description),Data(exif_get_short(entry->data,byteOrder),readableValue));
      break;

    case EXIF_FORMAT_LONG:
      exif_entry_get_value(entry,buf,1024);
      readableValue = buf;
      (*exifValues)[entry->tag] = QPair<Tag,Data>(Tag(title,description),Data(exif_get_long(entry->data,byteOrder),readableValue));
      break;

    case EXIF_FORMAT_RATIONAL:
      exif_entry_get_value(entry,buf,1024);
      readableValue = buf;
      rational = exif_get_rational(entry->data,byteOrder);
      value.setValue(rational);
      (*exifValues)[entry->tag] = QPair<Tag,Data>(Tag(title,description),Data(value,readableValue));
      break;

    case EXIF_FORMAT_SBYTE:
      exif_entry_get_value(entry,buf,1024);
      readableValue = buf;
      (*exifValues)[entry->tag] = QPair<Tag,Data>(Tag(title,description),Data(static_cast<int> (*entry->data),readableValue));
      break;

    case EXIF_FORMAT_UNDEFINED:
      break;

    case EXIF_FORMAT_SSHORT:
      exif_entry_get_value(entry,buf,1024);
      readableValue = buf;
      (*exifValues)[entry->tag] = QPair<Tag,Data>(Tag(title,description),Data(exif_get_sshort(entry->data,byteOrder),readableValue));
      break;

    case EXIF_FORMAT_SLONG:
      exif_entry_get_value(entry,buf,1024);
      readableValue = buf;
      (*exifValues)[entry->tag] = QPair<Tag,Data>(Tag(title,description),Data(exif_get_slong(entry->data,byteOrder),readableValue));
      break;

    case EXIF_FORMAT_SRATIONAL:
      exif_entry_get_value(entry,buf,1024);
      readableValue = buf;
      sRational = exif_get_srational(entry->data,byteOrder);
      value.setValue(sRational);
      (*exifValues)[entry->tag] = QPair<Tag,Data>(Tag(title,description),Data(value,readableValue));
      break;

    case EXIF_FORMAT_FLOAT:
      exif_entry_get_value(entry,buf,1024);
      readableValue = buf;
      (*exifValues)[entry->tag] = QPair<Tag,Data>(Tag(title,description),Data(*reinterpret_cast<float *> (entry->data),readableValue));
      break;

    case EXIF_FORMAT_DOUBLE:
      exif_entry_get_value(entry,buf,1024);
      readableValue = buf;
      (*exifValues)[entry->tag] = QPair<Tag,Data>(Tag(title,description),Data(*reinterpret_cast<double *> (entry->data),readableValue));
      break;
  }

  delete [] buf;
}

void Tags::readContent(ExifContent *content, void *data)
{
  exif_content_foreach_entry(content,readEntry,data);
}
#endif

Tags::Tags(const QFileInfo &fileInfo) : _fileName(fileInfo.absoluteFilePath())
{
#ifdef EXIF
  QFile file(fileName());

  file.open(QIODevice::ReadOnly);
  readExifData(file.readAll());
  file.close();
#endif
}

Tags::Tags(const QByteArray &data)
{
  readExifData(data);
}

void Tags::readExifData(const QByteArray &data)
{
#ifdef EXIF
  if (data.size())
  {
    ExifData *exifData = exif_data_new();
    unsigned char *buf;
    unsigned int bufSize;

    exif_data_load_data(exifData,reinterpret_cast<const unsigned char *> (data.constData()),data.size());
    exif_data_save_data(exifData,&buf,&bufSize);
    _data = QByteArray(reinterpret_cast<const char *> (buf),bufSize);
    byteOrder = exif_data_get_byte_order(exifData);
    exif_data_foreach_content(exifData,readContent,reinterpret_cast<void *> (this));
    exif_data_free(exifData);
    free(buf);
  }
#endif
}

QImage Tags::normalize(const QImage &image)
{
  QImage result = image;

#ifdef EXIF
  switch (value(EXIF_TAG_ORIENTATION).second.value.toInt())
  {
    case 1:
      break;

    case 2:
      result = result.mirrored(true,false);
      break;

    case 3:
      result = result.mirrored(true,true);
      break;

    case 4:
      result = result.mirrored(false,true);
      break;

    case 5:
      result = result.transformed(QTransform().rotate(90),Qt::SmoothTransformation).mirrored(true,false);
      break;

    case 6:
      result = result.transformed(QTransform().rotate(90),Qt::SmoothTransformation);
      break;

    case 7:
      result = result.transformed(QTransform().rotate(270),Qt::SmoothTransformation).mirrored(true,false);
      break;

    case 8:
      result = result.transformed(QTransform().rotate(270),Qt::SmoothTransformation);
      break;

    default:
      break;
  }
#endif

  return result;
}

InfoDialog::InfoDialog(const QFileInfo &fileInfo, QWidget *parent) : Gui::Dialog(Gui::Dialog::CenterOfScreen,parent)
{
  setupUi(Tags(fileInfo));
}

InfoDialog::InfoDialog(const QByteArray &data, QWidget *parent) : Gui::Dialog(Gui::Dialog::CenterOfScreen,parent)
{
  setupUi(Tags(data));
}

void InfoDialog::setupUi(const Tags &tags)
{
  int row = 0;
  QVBoxLayout *layout = new QVBoxLayout;

  layout->setContentsMargins(0,0,0,0);

  setWindowTitle(tr("EXIF information"));
  if (!tags.fileName().isEmpty())
    setWindowTitle(windowTitle() + " - " + QFileInfo(tags.fileName()).fileName());
  setWindowIcon(QIcon(":images/exif_info"));

  if (tags.size())
  {
    QMapIterator<int, QPair<Tag, Data> > tag(tags);
    QTableWidget *tableWidget = new QTableWidget(tags.size(),2,this);

    tableWidget->setAlternatingRowColors(true);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    tableWidget->setHorizontalHeaderItem(0,new QTableWidgetItem(tr("Tag")));
    tableWidget->setHorizontalHeaderItem(1,new QTableWidgetItem(tr("Value")));
    tableWidget->horizontalHeader()->setStretchLastSection(true);

    while (tag.hasNext())
    {
      QTableWidgetItem *item;
      QPair<Tag,Data> exifValue = tag.next().value();

      item = new QTableWidgetItem(exifValue.first.title);
      item->setToolTip(exifValue.first.description.split(". ",QString::SkipEmptyParts).join("\n"));
      tableWidget->setItem(row,0,item);
      item = new QTableWidgetItem(exifValue.second.readableValue);
      item->setToolTip(exifValue.first.description.split(". ",QString::SkipEmptyParts).join("\n"));
      tableWidget->setItem(row++,1,item);
    }

    tableWidget->resizeColumnsToContents();
    tableWidget->resizeRowsToContents();

    layout->addWidget(tableWidget);
  }
  else
  {
    QLabel *label = new QLabel(tr("No EXIF information available"),this);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
  }

#ifdef Q_WS_MAC
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok,Qt::Horizontal,this);

  connect(buttonBox,SIGNAL(accepted()),this,SLOT(close()));

  layout->addWidget(buttonBox);
#endif

  setLayout(layout);

  resize(600,400);
}
