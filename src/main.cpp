/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "ImageViewerApplication.h"

int main(int argc, char **argv){
	try{
		ImageViewerApplication app(argc, argv, "BorderlessViewer" + get_per_user_unique_id());
		return app.exec();
	}catch (ApplicationAlreadyRunningException &){
		return 0;
	}catch (NoWindowsException &){
		return 0;
	}
}
