#include "config.hpp"
namespace cfg{
	std::string MODEL_FILE = "/root/model/STMixer_5f.om";
	std::string IN = "/root/notebooks/final/data/test.mp4";
	// std::string OUT= "rtsp://192.168.137.101:8554/mystream";
	// std::string OUT_TYPE = "rtsp";
	std::string OUT= "/root/cpp-one/out.mp4";
	std::string OUT_TYPE = "file";
	int NFRAME=5;
	int HEIGHT=224;
	int WEITH=399;
	float THRESHOLD=0.15f;
	float BG_THRESHOLD=0.7f;
	int NQUERY=25;
	float SAMPLE = 2;
	std::string LABELS[]={"aerobic push up", "aerobic explosive push up", "aerobic explosive support", "aerobic leg circle", "aerobic helicopter", "aerobic support", "aerobic v support", "aerobic horizontal support", "aerobic straight jump", "aerobic illusion", "aerobic bent leg(s) jump", "aerobic pike jump", "aerobic straddle jump", "aerobic split jump", "aerobic scissors leap", "aerobic kick jump", "aerobic off axis jump", "aerobic butterfly jump", "aerobic split", "aerobic turn", "aerobic balance turn", "volleyball serve", "volleyball block", "volleyball first pass", "volleyball defend", "volleyball protect", "volleyball second pass", "volleyball adjust", "volleyball save", "volleyball second attack", "volleyball spike", "volleyball dink", "volleyball no offensive attack", "football shoot", "football long pass", "football short pass", "football through pass", "football cross", "football dribble", "football trap", "football throw", "football diving", "football tackle", "football steal", "football clearance", "football block", "football press", "football aerial duels", "basketball pass", "basketball drive", "basketball dribble", "basketball 3-point shot", "basketball 2-point shot", "basketball free throw", "basketball block", "basketball offensive rebound", "basketball defensive rebound", "basketball pass steal", "basketball dribble steal", "basketball interfere shot", "basketball pick-and-roll defensive", "basketball sag", "basketball screen", "basketball pass-inbound", "basketball save", "basketball jump ball"};
	int NLABEL=66;
	int NCLASS=67;
};