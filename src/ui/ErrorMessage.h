//
// Created by Tobias Hieta on 18/03/16.
//

#ifndef PLEXMEDIAPLAYER_ERRORMESSAGE_H
#define PLEXMEDIAPLAYER_ERRORMESSAGE_H

#include <QMessageBox>

class ErrorMessage : public QMessageBox
{
  Q_OBJECT
public:
  explicit ErrorMessage(const QString& errorMessage, bool allowResetConfig = false);
};

#endif //PLEXMEDIAPLAYER_ERRORMESSAGE_H
