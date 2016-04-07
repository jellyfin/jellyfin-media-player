#ifndef VERSION_H
#define VERSION_H

namespace Version
{
  QString GetVersionString();
  QString GetCanonicalVersionString();
  QString GetBuildDate();
  QString GetWebVersion();
  QString GetQtDepsVersion();
  QString GetDependenciesVersion();
};

#endif // VERSION_H

