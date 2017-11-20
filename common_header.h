/*********************************************************************
 * Header file    common_header.h
 * Project:       Мегафон 2
 * Purpose:       Общий с ПО верхнего уровня
 * Created:       07-10-02 (1) 14:34 by K.Kapranov (C) NITA
 * Comments:
 ********************************************************************/
#ifndef _common_header_h_
#define _common_header_h_

#define IO_PERIOD       8 //8mS периодичность обмена аудиоданными с верхним уровнем (коилчество пакетов за одно обращение)
#define CRATE_COUNT     2 //2 крейта
#define CHANNEL_COUNT   32 //32 канала в крейте
#define SAMPLE_FREQ     8 //8 сэмплов в миллисекунду

/*
typedef struct{//Для файлового В/В
        u8      Sound[CRATE_COUNT][CHANNEL_COUNT][IO_PERIOD][SAMPLE_FREQ];//Аудиоданные
        s64     Signal[CRATE_COUNT][IO_PERIOD/2];//Сигнализация
        s64     Ctrl[CRATE_COUNT][IO_PERIOD/2];//состояние крейта
        } FileXchange;
*/
#define EMAN 125 //Хотя бы один из крейтов в ручном режиме или в ручном режиме по кнопке

#endif /* end of file common_header.h */
