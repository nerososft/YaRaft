//
// Created by XingfengYang on 2020/1/7.
//

#ifndef RAFT_LOG_H
#define RAFT_LOG_H

#include <stdio.h>

#define DEBUG1(fmt, arg  ...)  \
    do{printf("[DEBUG] " fmt ,  ##arg );  }while(0);

#define DEBUG2(fmt, arg  ...) \
    do{printf("[%s: %s: line %d]" fmt ,\
        __FILE__, __FUNCTION__, __LINE__,  ##arg );  }while(0);

#define PrintColor1(fmt, arg  ...)    \
    do{printf("\033[30m""[%s: %s: line %d]" fmt"\033[0m" ,\
        __FILE__, __FUNCTION__, __LINE__,  ##arg );  }while(0);

#define PrintColor2(fmt, arg  ...)    \
    do{printf("\033[31m""[%s: %s: line %d]" fmt"\033[0m" ,\
        __FILE__, __FUNCTION__, __LINE__,  ##arg );  }while(0);

#define PrintColor3(fmt, arg  ...)    \
    do{printf("\033[32m""[%s: %s: line %d]" fmt"\033[0m" ,\
        __FILE__, __FUNCTION__, __LINE__,  ##arg );  }while(0);

#define PrintColor4(fmt, arg  ...)    \
    do{printf("\033[33m""[%s: %s: line %d]" fmt"\033[0m" ,\
        __FILE__, __FUNCTION__, __LINE__,  ##arg );  }while(0);

#define PrintColor5(fmt, arg  ...)    \
    do{printf("\033[34m""[%s: %s: line %d]" fmt"\033[0m" ,\
        __FILE__, __FUNCTION__, __LINE__,  ##arg );  }while(0);

#define PrintColor6(fmt, arg  ...)    \
    do{printf("\033[35m""[%s: %s: line %d]" fmt"\033[0m" ,\
        __FILE__, __FUNCTION__, __LINE__,  ##arg );  }while(0);

#define PrintColor7(fmt, arg  ...)     \
    do{printf("\033[36m""[%s: %s: line %d]" fmt"\033[0m" ,\
        __FILE__, __FUNCTION__, __LINE__,  ##arg );  }while(0);


#define NormalPrintColor1(fmt, arg  ...)    \
    do{printf("\033[30m" fmt"\033[0m" ,##arg );  }while(0);

#define NormalPrintColor2(fmt, arg  ...)    \
    do{printf("\033[31m" fmt"\033[0m" ,##arg );  }while(0);

#define NormalPrintColor3(fmt, arg  ...)    \
    do{printf("\033[32m" fmt"\033[0m" ,##arg );  }while(0);

#define NormalPrintColor4(fmt, arg  ...)    \
    do{printf("\033[33m" fmt"\033[0m" ,##arg );  }while(0);

#define NormalPrintColor5(fmt, arg  ...)    \
    do{printf("\033[34m" fmt"\033[0m" ,##arg );  }while(0);

#define NormalPrintColor6(fmt, arg  ...)    \
    do{printf("\033[35m" fmt"\033[0m" ,##arg );  }while(0);

#define NormalPrintColor7(fmt, arg  ...)     \
    do{printf("\033[36m" fmt"\033[0m" , ##arg );  }while(0);

#define LogError(fmt, arg  ...)    \
    do{printf("\033[31m" fmt"\033[0m" ,##arg );  }while(0);

#define LogInfo(fmt, arg  ...)    \
    do{printf("\033[32m" fmt"\033[0m" ,##arg );  }while(0);

#define LogWarnning(fmt, arg  ...)    \
    do{printf("\033[33m" fmt"\033[0m" ,##arg );  }while(0);

#endif //RAFT_LOG_H
