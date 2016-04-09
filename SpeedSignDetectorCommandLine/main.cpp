// Qt Includes
#include <QApplication>
#include <QCommandLineParser>
#include <QTextStream>
#include <QTimer>

// SpeedSignDetector Includes
#include "detectortask.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  setlocale(LC_NUMERIC,"C");

  QCommandLineParser parser;
  parser.setApplicationDescription("Speed Sign Detector Command Line");
  parser.addHelpOption();

  QCommandLineOption trainingFileOption(QStringList() << "t" << "training-file",
          "Use <file> for training the detection.",
          "trainingFile");
  parser.addOption(trainingFileOption);

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

  QString trainingFile = parser.value(trainingFileOption);
  QString targetFile = parser.value(targetFileOption);
  QString resultFile = parser.value(resultFileOption);
  bool colorElimination = parser.isSet(colorEliminationOption);
  bool verbose = parser.isSet(verboseOption);

  QTextStream out(stdout);

  if (trainingFile.isEmpty()) {
    out << "A training file is required, give one using --training-file <file>." << endl;
    return 1;
  }

  if (targetFile.isEmpty()) {
    out << "A target file is required, give one using --target-file <file>." << endl;
    return 2;
  }

  DetectorTask *task = new DetectorTask(&a);
  task->setTrainingFile(trainingFile);
  task->setTargetFile(targetFile);
  task->setResultFile(resultFile);
  task->setColorElimination(colorElimination);
  task->setVerbose(verbose);

  QObject::connect(task, SIGNAL(finished()), &a, SLOT(quit()));

  QTimer::singleShot(0, task, SLOT(run()));

  return a.exec();
}
