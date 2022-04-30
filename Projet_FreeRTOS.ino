#include <Arduino_FreeRTOS.h>
#include <semphr.h>

//Declaration (structure)
struct valeurCapteurs{
    int analogique;
    int numerique;
    double tempsEnMillisecondes; 
};

//Semaphore et queues
SemaphoreHandle_t xSemaphoreSerie = NULL; // sert à gérer le port série et être sûre qu'une seule tâche y accède
QueueHandle_t Pt_q; //potentiometer
QueueHandle_t Bt_q; //Somme des 2 bouttons poussoir
QueueHandle_t StructCapt_q; 
QueueHandle_t Struct2_q; // utilisé par 4ème tâche pour envoyé les data
//Les Ports
int Pt = A0; 
int Bt1 = 2; 
int Bt2 = 3; 

const TickType_t xdelay = 100 / portTICK_PERIOD_MS; 

void setup() {
  Serial.begin(9600); 
  while (!Serial) {
    ; // Attendre la connexion du port série 
  }

  if ( xSemaphoreSerie == NULL ) //Vérifiez  que la sémaphore en série n'a pas été déja créer
  {
    xSemaphoreSerie = xSemaphoreCreateMutex(); 
    if ( ( xSemaphoreSerie ) != NULL )// Vérifiez  que la sémaphore en série a déjà été créer
      xSemaphoreGive( ( xSemaphoreSerie ) );  // port série disponible pour l'utilisation
  }
  
// Initialisation 
  pinMode(Pt, INPUT);
  pinMode(Bt1, INPUT); 
  pinMode(Bt2, INPUT);
  
//Creation des taches
  xTaskCreate(Task1,"READ_ANALOG",128,NULL,1,NULL);
  xTaskCreate(Task2,"READ_BUTT",128,NULL,1,NULL);
  xTaskCreate(Task3,"Struct",128,NULL,1,NULL);
  xTaskCreate(Task4,"Affichage port série",1000,NULL,1,NULL);
  xTaskCreate(Task5,"TempsEnMin et affichage",1000,NULL,1,NULL);
  
//Initialisation des Queues
  Pt_q = xQueueCreate(5,sizeof(uint32_t));
  Bt_q = xQueueCreate(5,sizeof(uint32_t));
  StructCapt_q = xQueueCreate(5,sizeof(valeurCapteurs)); 
  Struct2_q = xQueueCreate(5,sizeof(valeurCapteurs));
  
}
void loop() {}

//Tache 1
void Task1(void *pvParameters)
{
  int val_Pt = 0;
  while(1)
  {
    val_Pt= analogRead(Pt); // Récupèrer la valeur analogique appliqué sur A0
    xQueueSend(Pt_q, &val_Pt, 0); // valeur attribué à la Queue Pt_q
    vTaskDelay( xdelay ); 
  }
}
//Tache 2
void Task2(void *pvParameters)  
{
  int val;
  while(1){
    val = digitalRead(Bt1)+digitalRead(Bt2); // Somme des valeurs des deux entré numérique 3 et 4
    xQueueSend(Bt_q,&val,0);
    vTaskDelay( xdelay  ); 
  }
  
}
//Tache 3
void Task3(void *pvParameters)
{
  int Val_Pt;
  int Val_Bt;
  valeurCapteurs capteur;
  while(1)
  {
    
    xQueueReceive(Pt_q,&Val_Pt,0); //valeurs de la tâche 1
    xQueueReceive(Bt_q,&Val_Bt,0); //valeurs de la tâche 2

    capteur.analogique = Val_Pt; // valeur de la tâche 1 (data du potentiometre) affecté à la structure 
    capteur.numerique = Val_Bt;  // valeur de la tâche2 (Somme des entrées Num3 et Num4) affecté à la structure 
    capteur.tempsEnMillisecondes = millis();
    xQueueSend(StructCapt_q,&capteur,0); // valeur envoyé à la Queue StructCapt_q
    vTaskDelay( xdelay  );

  }
}

//Tache 4
void Task4(void *pvParameters)
{
  valeurCapteurs capteur;
  while(1)
  {
    xQueueReceive(StructCapt_q,&capteur,0); //valeur reçu dans la queue StructCapt_q
    
    if ( xSemaphoreTake( xSemaphoreSerie, ( TickType_t ) 5 ) == pdTRUE ) // accès à la ressource partagée
    {
      //Affichage
      Serial.print("Structure DATA = { ");
      Serial.print(capteur.analogique);
      Serial.print(" ; ");
      Serial.print(capteur.numerique);
      Serial.print(" ; ");
      Serial.print(capteur.tempsEnMillisecondes);
      Serial.println(" }");
      xSemaphoreGive( xSemaphoreSerie ); // Libérer le port série
    }
    xQueueSend(Struct2_q,&capteur,0); //valeur envoyé à la Queue Struct2_q
    vTaskDelay( xdelay  );
  }
}

//Tache 5

void Task5(void *pvParameters)
{
  valeurCapteurs capteur;
  valeurCapteurs capteur2;
  while(1)
  {
    xQueueReceive(Struct2_q,&capteur,0); // Valeurs reçu de la tâche 4
    capteur2.analogique = capteur.analogique;
    capteur2.numerique = capteur.numerique;
    capteur2.tempsEnMillisecondes = capteur.tempsEnMillisecondes/60000; //conversion en minutes
    
    
    if ( xSemaphoreTake( xSemaphoreSerie, ( TickType_t ) 5 ) == pdTRUE ) // Vérifier si on peut accéder au port série
    {
      //Affichage
      Serial.print("New structure DATA = { ");
      Serial.print(capteur2.analogique);
      Serial.print(" ; ");
      Serial.print(capteur2.numerique);
      Serial.print(" ; ");
      Serial.print(capteur2.tempsEnMillisecondes);
      Serial.println(" }");

      xSemaphoreGive( xSemaphoreSerie ); // Libéreration
    }

    vTaskDelay( xdelay );
  }
}
