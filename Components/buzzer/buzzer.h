#ifndef _BUZZER_H_
#define _BUZZER_H_ 


typedef enum
{
   BUZZER_PROMPT_NONE = 0,
   BUZZER_PROMPT_KEY ,
   BUZZER_PROMPT_ICCARD ,

}BUZZER_PROMPT_TYPE_E;


void buzzer_prompt_load(BUZZER_PROMPT_TYPE_E prompt_type);

void buzzer_task(void *param);


#endif