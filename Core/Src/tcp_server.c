//#include "tcp_server.h"
//#include "FreeRTOS.h"
//#include "task.h"
//#include "usart.h"
//
//void tcpReceiveTask(void *pvParameters) {
//    int sock;
//    struct sockaddr_in serverAddr, clientAddr;
//    int addr_len = sizeof(clientAddr);
//    char recvBuffer[128];
//    
//    // �����׽���
//    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
//        // �������
//        vTaskDelete(NULL);
//    }
//
//    // ���÷�������ַ
//    serverAddr.sin_family = AF_INET;
//    serverAddr.sin_port = htons(5000);
//    serverAddr.sin_addr.s_addr = INADDR_ANY;
//
//    // ���׽���
//    if (bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
//        // �������
//        close(sock);
//        vTaskDelete(NULL);
//    }
//
//    // ��������
//    if (listen(sock, 1) < 0) {
//        // �������
//        close(sock);
//        vTaskDelete(NULL);
//    }
//
//    while (1) {
//        int newSock = accept(sock, (struct sockaddr *)&clientAddr, (socklen_t *)&addr_len);
//        if (newSock < 0) {
//            // �������
//            continue;
//        }
//
//        while (1) {
//            int recvLen = recv(newSock, recvBuffer, sizeof(recvBuffer) - 1, 0);
//            if (recvLen > 0) {
//                recvBuffer[recvLen] = '\0';
//                HAL_UART_Transmit(&huart1, (uint8_t*)recvBuffer, recvLen, HAL_MAX_DELAY);
//            } else {
//                // �������ӶϿ�
//                break;
//            }
//        }
//        close(newSock);
//    }
//}
