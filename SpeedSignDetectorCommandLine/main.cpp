// Qt Includes
#include <QApplication>
#include <QCommandLineParser>
#include <QTextStream>
#include <QTimer>
#include <QFileInfo>

// SpeedSignDetector Includes
#include "detectortask.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  setlocale(LC_NUMERIC,"C");

  QCommandLineParser parser;
  parser.setApplicationDescription("Speed Sign Detector Command Line");
  parser.addHelpOption();

  QCommandLineOption modeOption(QStringList() << "m" << "mode",
          "Choose <mode> between \"Edge\" or \"Harris\".",
          "mode");
  parser.addOption(modeOption);

  QCommandLineOption trainingDirectoryOption(QStringList() << "t" << "training-directory",
          "Use <trainingDirectory> for training the detection.",
          "trainingDirectory");
  parser.addOption(trainingDirectoryOption);

  QCommandLineOption targetFileOption(QStringList() << "a" << "target-file",
          "Look for object in <targetFile>.",
          "targetFile");
  parser.addOption(targetFileOption);

  QCommandLineOption resultFileOption(QStringList() << "r" << "result-file",
          "Save targeted image as <resultFile>.",
          "resultFile");
  parser.addOption(resultFileOption);

  QCommandLineOption colorEliminationOption(QStringList() << "e" << "eliminate-colors",
          "Eliminate uninteresting colors first.");
  parser.addOption(colorEliminationOption);

  QCommandLineOption verboseOption(QStringList() << "v" << "verbose",
          "Verbose output.");
  parser.addOption(verboseOption);

  parser.process(a);

  QString mode = parser.value(modeOption);
  QString trainingDirectory = parser.value(trainingDirectoryOption);
  QString targetFile = parser.value(targetFileOption);
  QString resultFile = parser.value(resultFileOption);
  bool colorElimination = parser.isSet(colorEliminationOption);
  bool verbose = parser.isSet(verboseOption);

  QTextStream out(stdout);

  if (trainingDirectory.isEmpty()) {
    out << "A training directory is required, give one using --training-directory <directory>." << endl;
    return 1;
  }

  if (!QFileInfo(trainingDirectory).isDir()) {
    out << "The training directory must be a directory." << endl;
    return 2;
  }

  if (targetFile.isEmpty()) {
    out << "A target file is required, give one using --target-file <file>." << endl;
    return 3;
  }

  DetectorTask *task = new DetectorTask(&a);
  task->setMode(mode);
  task->setTrainingDirectory(trainingDirectory);
  task->setTargetFile(targetFile);
  task->setResultFile(resultFile);
  task->setColorElimination(colorElimination);
  task->setVerbose(verbose);

  QObject::connect(task, SIGNAL(finished()), &a, SLOT(quit()));

  QTimer::singleShot(0, task, SLOT(run()));

  return a.exec();
}
