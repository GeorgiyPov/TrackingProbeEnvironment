#include "register_settings.h"
#include <QSettings>
#include <QFile>
#include <QDir>

static const char* TAG_NAME{"[ SETTINGS ]"};

std::map<QString,QString>                SettingsRegister::settings;
std::map<QString,float>                  SettingsRegister::settings_values;
std::map<QString,std::pair<float,float>> SettingsRegister::settings_pairs;

bool SettingsRegister::FLAG_SETTINGS_LOADED = false;

void SettingsRegister::LoadSettings()
{
  QString last_path;

  auto CheckPath = [&last_path](QString path) -> bool
  {
    QFile file(path);  last_path = path;

     if(path.isEmpty()) return false;
    if(!path.contains("TrackingSettings.ini")) return false;
    return file.exists(); 
  };
  FLAG_SETTINGS_LOADED = true;

  QStringList params; 
              params << "/home" << "-maxdepth" << "4" << "-type" << "f" << "-name" << "TrackingSettings.ini";

                                QProcess findCommand;
                                         findCommand.start("/usr/bin/find",params); 
                                         findCommand.waitForFinished(1000);
                    QString result_str = findCommand.readAll(); 
  QStringList result_list = result_str.split("\n");

  QString env_Path = qgetenv("TRACKING_SETTINGS_PATH");
  QString currentPath = QDir::currentPath() + "/DATA/TrackingProject/TrackingSettings.ini";
  QString homePath = QString("/home/broms/DATA/TrackingProject/TrackingSettings.ini");
  QString homePathWin = QString("D:/DATA/TrackingProject/TrackingSettings.ini");



         std::vector<QString>   LocationList;
  if(CheckPath(homePathWin))    LocationList.push_back(last_path);
  if(CheckPath(homePath)   )    LocationList.push_back(last_path);
  //if(CheckPath(currentPath))    LocationList.push_back(last_path);
  //if(CheckPath(result_list[0])) LocationList.push_back(last_path);

  qDebug() << "=================================";
  for(auto& path: LocationList) qDebug() << " EXISTS: " << path;
  qDebug() << "=================================" << Qt::endl;

  bool result = false;
  for(auto& path: LocationList) 
  { result = TryLoadSettings(path, "PATHS"); if(result) break; }

    result = TryLoadSettings(SettingsRegister::GetString("FILE_PORTS"),"PORTS");

  AppendSettings("CAMERA_IMAGE_POS",  std::pair<float,float>(20.0,20.0));
  AppendSettings("CAMERA_IMAGE_SIZE", 400.0);
  AppendSettings("CAMERA_IMAGE_SIZE", std::pair<float,float>(400.0,400.0));
  AppendSettings("DATA_DIR", "D:/DATA");

  //SCANATOR +-10V -> +-14 degree
  float ANGLE_RANGE = 2*60*60;
  float  VOLT_RANGE = 3;
  float   DAC_RANGE = std::pow(2.0,12.0);
  float   PIX_RANGE = 400;

  float PIX_TO_ANGLE = ANGLE_RANGE/PIX_RANGE;
  float ANGLE_TO_DAC = DAC_RANGE/ANGLE_RANGE;
  //float PIX_TO_DAC   = DAC_RANGE*PIX_RANGE;
  float PIX_TO_DAC   = DAC_RANGE/PIX_RANGE;

  AppendSettings("GAIN_PIXEL_ANGLE",PIX_TO_ANGLE);
  AppendSettings("GAIN_PIXEL_DAC"  ,PIX_TO_DAC);
  AppendSettings("GAIN_ANGLE_DAC"  ,ANGLE_TO_DAC);
  AppendSettings("DAC_OFFSET", 0);

  AppendSettings("BLOCK_ID_CAMERA"  , 10);
  AppendSettings("BLOCK_ID_IMG_PROC", 20);
  AppendSettings("BLOCK_ID_AIMING"  , 30);
  AppendSettings("BLOCK_ID_SCANATOR", 40);
  AppendSettings("BLOCK_ID_IMITATOR", 50);

  qDebug() << "============================";
  qDebug() << "[ LOADED RECORDS ]";
  PrintSettingsList();
  qDebug() << "============================";
  //===========================================
}

bool SettingsRegister::TryLoadSettings(QString file, QString GROUP)
{
  if(!QFile::exists(file)) return false;;

  QSettings base_settings(file, QSettings::IniFormat);

            base_settings.beginGroup(GROUP);
                  for(auto key: base_settings.allKeys())
                  settings.emplace(key, base_settings.value(key).toString() );
            base_settings.endGroup();

  qDebug() << TAG_NAME << "LOADED: " << file << "GROUP: " << GROUP;
  return true;
}

void SettingsRegister::AppendSettings(QString key, QString setting)                { settings.emplace(key,setting); }
void SettingsRegister::AppendSettings(QString key, float setting)                  { settings_values.emplace(key,setting); }
void SettingsRegister::AppendSettings(QString key, std::pair<float,float> setting) { settings_pairs.emplace(key,setting); }
