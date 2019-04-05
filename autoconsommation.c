//--------PROGRAMME POUR OPTIMISER AUTOCONSOMMATION PHOTOVOLTAIQUE---version  qu'une sonde de température avril 2019-----------
//---1ere partie c'est la  mesure des valeurs Volt_1 et Ampere_1---------------------------------------------------
//---2eme partie, Calculer les watts instantanés,les wattheures cumulés, les kWh et envoyer les valeurs sur un écran LCD
//---3eme partie, Gerer le servo moteur --------------------------------------------------------------------------
//---4eme Partie, Pour 2 autres pinces amperemetriques------------------------------------------------------------
//---5eme partie, gestion de differents affichages avec bouton poussoir sur un meme ecran LCD 2 lignes ---------------
//---6eme partie , remplace la partie 3 du servo moteur 
//-- 7eme partie mesure et affichage de la température

#include "EmonLib.h"             // 1 Pour inclure la librairie "EmonLib.h", aller dans le menu "croquis" puis "inclure une bibliothèque" 
                                 // 1 puis "gérer les bibliothèques ". Une fois la librairie trouvée, appuyer sur "More info" puis " Installer"
EnergyMonitor emon1;             // 1 Creer une instance qui s'appelle emon1 (on s'en servira si on veut installer plusieurs pinces ampèremétriques)
EnergyMonitor emon2;             // 4 Idem que dessus pour une deuxième pince amperemetrique
EnergyMonitor emon3;             // 4 Idem que dessus pour une troisième pince amperemetrique

#include <Wire.h>                        // 2 inclure la librairie "wire.h" (si elle n'existe pas dans la bibliothèque, faire comme expliqué pour EmonLib.h
#include <LiquidCrystal_I2C.h>           // 2 inclure la librairie "LiquidCrystal_I2C.h" 
//LiquidCrystal_I2C lcd(0x27, 16, 4);    // 2 définir l'adresse de l'écran LCD, le nombre de caractères et le nombre de lignes
LiquidCrystal_I2C lcd(0x3f,16,2);        // 2 Si Le LCD ne fonctionne pas, alors mettre // sur la ligne précédente et enlever les // sur cette ligne

#include <DallasTemperature.h>           // 7 librairie de la sonde Dallas
#include <OneWire.h>                     // 7 librairie qui va gérer les différente adresses des sondes de température

//#include <Servo.h>                     // 3 inclure la librairie "Servo.h" 
//Servo myservo;                         // 3 Création de l'objet Servo pour controler Servo
//float pos = 0;                         // 3 Creation de la variable (flottante)qui donnera la position du servo
const int led = 5;                       // 6  valeur finale auquel la led va s'allumer  de 0(éteint) à 250 (allumé à fond)
int intensite_led = 0;                   // 6  Variable intermédiaire  de l'intensité de la led

float w_instantane_in = 0;            // 2 Creation de la variable flottante "puissance instantanée" qui rentre (en watt) initialisée à la valeur 0
float kwh_cumule_in=0;                // 2' Création de la variable flottante correspondant au Kilo Watt heure "consommé" cumulés initialisé à 0

float w_instantane_out = 0;           // 2 Creation de la variable flottante "puissance instantanée" qui sort (negative) en watt initialisée à la valeur 0
float kwh_cumule_out=0;               // 2' Création de la variable flottante correspondant au Kilo Watt heure "surproduction" cumulés initialisé à 0

float w_Solaire = 0;                 // 4 Creation de la variable flottante "puissance instantanée2" 2ième pince ampèrmétrique, initialisée à la valeur 0
float kwh_Solaire = 0;               // 4 Creation de la variable flottante "puissance cumulée2" qui rentre en Kilowattheure initialisée à la valeur 0

float w_Chauffe_Eau = 0;             // 4 Creation de la variable flottante "puissance instantanée2" 2ième pince ampèrmétrique, initialisée à la valeur 0
float kwh_Chauffe_Eau = 0;           // 4 Creation de la variable flottante "puissance cumulée2" qui rentre en Kilowattheure initialisée à la valeur 0

const int pin_bouton_changer_ecran=9;  //5 numéro de la pin du bouton poussoir qui sert à changer d'écran
int impulsion = 0;                     //5 creation de la variable impulsion
int compteur_impulsion = 1;            //5 compteur du nombre d'impulsion effectué sur le bouton poussoir, variable qui va servir pour le changement d'affichage
                 

unsigned long previous_millis = 0;   // 2 création de la variable "previous_millis" qui garde en mémoire le temps qui s'écoule en millièmes de seconde"

const int SONDE=12;          // 7 numéro de broche sur l'arduino pour la temperature
float Temp=0 ;               // 7 création d'une variable temporaire de la températude du ballon d'eau chaude
float TEMPERATURE0 = 0;      // 7 variable flotante pour récupérer la température 
long Compteur=0;             // 7 initialisation du compteur qui décompte le temps écoulé pendant lequel l'eau stagne sous les 55°C

OneWire ONE_WIRE_BUS (SONDE);              // 7 création de l'objet ONE_WIRE_BUS1 sur la pin SOND_1 (48 dans mon cas)
DallasTemperature sensor(&ONE_WIRE_BUS);   // 7 utilisation du bus OneWire
DeviceAddress sensor1DeviceAddress;        // 7 déclaration de "sensor1DeviceAddress" comme adresse de la sonde 1

//-----------------------INITIALISATION DU PROGRAMME-------------------------------------------------

void setup()
{
  Serial.begin(9600);                 // 1 Création du port série pour que l'arduino puisse envoyer des infos vers l'ordinateur

  emon1.voltage(0, 357, 1.7);         // 1 Initialisation du Voltage (Pin A2, Valeur à changer pour etalonnage (+/-357 pour 6v et +/- 190 pour 12v))
  emon1.current(1, 28.7);             // 1 Initialisation du Courant en ampère ( Pin A1, Valeur à changer lors de l'etalonnage)
  
  emon2.voltage(0, 357, 1.7);         // 4 Initialisation du Voltage (Pin A2, Valeur à changer pour etalonnage, phase_shift)
  emon2.current(2, 28.6);             // 4 Initialisation du Courant en ampère ( Pin A3, Valeur à changer lors de l'etalonnage)
  
  emon3.voltage(0, 357, 1.7);         // 4 Initialisation du Voltage (Pin A2, Valeur à changer pour etalonnage, phase_shift)
  emon3.current(3, 28.5);             // 4 Initialisation du Courant en ampère ( Pin A6, Valeur à changer lors de l'etalonnage)

  lcd.init();                         // 2 initialisation de l'afficheur LCD 

 //myservo.attach(5);                 // 3 On attache l'objet servo à la Pin D5 
 pinMode(led, OUTPUT);                // 6 La broche sur lequel est la variable led est une sortie 

 pinMode(pin_bouton_changer_ecran, INPUT_PULLUP); // 5 Le bouton poussoir est une entrée forcé à 0V si on appuis pas et à 5V  si on appuie

  sensor.begin();                                 // 7 Activation du capteur de temperature
  sensor.getAddress(sensor1DeviceAddress, 0);     // 7 Demande l'adresse du capteur à l'index 0
  sensor.setResolution(sensor1DeviceAddress, 12); // 7 Résolution 12 bits
}

 //----------------------- DEMARRAGE DE LA BOUCLE----------------------------------------------------

void loop()
{
  float puissance_reelle1 = emon1.realPower;   // 1 creation de la variable flottante "puissance reelle" qui existe dans la librairie sous "emon1.realPower"
  float puissance_reelle2 = emon2.realPower;   // 4 creation de la variable flottante "puissance reelle" qui existe dans la librairie sous "emon1.realPower"
  float puissance_reelle3 = emon3.realPower;   // 4 creation de la variable flottante "puissance reelle" qui existe dans la librairie sous "emon1.realPower"

  float verif_voltage    = emon1.Vrms;        // 1 creation de la variable "volts moyen" (mesurable avec un voltmètre pour l'etalonnage)
  float verif_ampere     = emon1.Irms;        // 1 creation de la variable "Ampères Moyen" (mesurable avec une pince ampèremétrique pour l'etalonnage))
  float Cos_phi          = emon1.powerFactor;

  emon1.calcVI(20,2000);                      // 1 Demande a Emonlib de tout calculer,  (puissance relle, volts moyen, ampère moyen et facteur de puissance)
  emon2.calcVI(20,2000);                      // 4 idem qu'au dessus mais pour la deuxième pince ampèrmétrique (si on met 40 au lieu de 20, ca va moins vite)
  emon3.calcVI(20,2000);                      // 4 idem qu'au dessus mais pour la troisième pince ampèrmétrique (et du coup c'est plus lisible sur le LCD)
  //emon1.serialprint();                      //(1) Si on ecrit cette ligne , toutes les valeurs calculées precedemment sont envoyées vers l'ordinateur
  
  sensor.requestTemperatures();               // 7 Demande la température du capteur
  Temp=sensor.getTempCByIndex(0);             // 7 Récupération de la température en celsius à l'index 0

 //--------------------------Etalonnage des volts et ampères sans LCD--------------------------------------

  Serial.print("Est-ce le bon voltage? ");      // 1 envoyer vers l'ordinateur le texte " Est-ce le bon voltage? "
  Serial.print(verif_voltage);                  // 1 envoyer vers l'ordinateur la valeur "verif_voltage (Vrms)"
  Serial.print(" V  ");                         // 1 envoyer vers l'ordinateur le caractère "V"
  Serial.print(verif_ampere);                   // 1 envoyer vers l'ordinateur la valeur "verif_voltage (Vrms)"                    
  Serial.print(" A ");                          // 1 envoyer vers l'ordinateur le caractère "A"
  Serial.print(Cos_phi);                   // 1 envoyer vers l'ordinateur la valeur "verif_voltage (Vrms)"                    
  Serial.print(" cos ");                          // 1 envoyer vers l'ordinateur le caractère "A"
     
 //----------------POUR AVOIR LES W, Wh et kWh de l'élélectricité qui rentre et de l'électricité qui sort de ma maison------------------
  
  if(puissance_reelle1 >= 0)                       // 2 Si la puissance reelle est positive, (c'est que je consomme et qu'a priori il n'y a pas de soleil)
        { 
         w_instantane_in = puissance_reelle1;      // 2 alors on dit que la puissance instantanée entrante (in) est egale à la puissance reelle.
         w_instantane_out = 0;                     // 2 dans ces conditions de consommation (positive) , la valeur de la surproduction est nulle.

         kwh_cumule_in = kwh_cumule_in + puissance_reelle1 * (millis() - previous_millis) / 3600000/1000;
                                                   // 2 La valeur cumulée consommée (entrante) = La valeur cumulée consommée précédente, 
                                                   // 2 plus la puissance reelle multipliée par le temps écoulé entre millis et previous millis
                                                   // 2 que divise 3600 (nb secondes / heure) et encore par 1000 car millis compte les millièmes de seconde                                               
                                                   // 2 et encore par mille car ce sont les kWh et non les Wh qui m'intêresse.
        }
  
  else                                             // 2 SINON (c'est que la puissance_reelle est négative)
        {
        w_instantane_in = 0;                       // 2 idem au dessus
        w_instantane_out = puissance_reelle1;      // 2 idem au dessus
    
        kwh_cumule_out = kwh_cumule_out + puissance_reelle1 * (millis() - previous_millis) / 3600000/ 1000; 
        }
     
// ----------------------Pour avoir les w, wh et kwh production solaire et consommation chauffe eau------

    w_Solaire = puissance_reelle2;            //4 idem au dessus mais pour sa pince ampèremetrique branché sur l'arrivée d'electricité solaire
    kwh_Solaire = kwh_Solaire + puissance_reelle2 * (millis() - previous_millis) / 3600000/1000; 
   
    w_Chauffe_Eau = puissance_reelle3;        //4 idem au dessus mais la pince ampèremetrique branché sur la conso du chauffe eau
    kwh_Chauffe_Eau = kwh_Chauffe_Eau + puissance_reelle3 * (millis() - previous_millis) / 3600000/ 1000;
         

    previous_millis = millis();               // 2 Demandons a la variable previous_millis de garder en memoire la valeur de millis (le tps qui s'écoule)

/*--------------- POUR FAIRE VARIER LA POSITION DU SERVO MOTEUR SUIVANT LA PUISSANCE CONSOMMEE---------

  if (puissance_reelle1 <= 1 && pos < 165)       // 3 comprendre (si on consomme très peu ( moins de 50W) et que le servo n'est pas à sa valeur max
    {pos += 5;  myservo.write(pos);}             // 3 alors augmenter la consommation en tournant vers la droite la molette du variateur de 5 degres

  if (puissance_reelle1 >= 100 && pos > 10)      // 3 Inversement si on consomme beaucoup (plus de 50W) et que le servomoteur n'est pas à son minimum
       {pos -= 5;myservo.write(pos);}            // 3 Alors diminuer la consommation en tournant vers la gauche la molette du variateur de 5 degrés

 */      
//--------------- POUR FAIRE VARIER L'INTENSITE LUMINEUSE DE LA LED SUIVANT LA PUISSANCE CONSOMMEE DESIREE et la temperature---------

       if (Temp>0)                     //7 pour éviter les sauts de valeur négative de la sonde dallas (genre artefact)
       {TEMPERATURE0=Temp;}            //7 si la temp >0 on récupère la valeur de la température sinon laisser l'artefact de coté

       Compteur= Compteur+1;           //7 le comptage du temps commence
      
//-------------------------------------Si il y a des Watts , balance en dans le chauffe eau---------------------------------

  if (puissance_reelle1 <= 1 && intensite_led < 230) //6 si la puissance sonsommée est inférieur à 1w, et que la led n'est mas à son maximum
  {intensite_led += 5;                               // 6 augmenter l'intensité lumineuse de 5
  analogWrite(led, intensite_led);}                  //6 la led va s'éclairer au niveau de la valeur de intensite_led
  
  if(puissance_reelle1 >= 100 && intensite_led > 5)  //6 inversement, si la conso en watt est supérieur à 100 et et que la led n'est pas à son minimum
  {intensite_led -= 5;                               //6 diminue alors l'intensité de la led
  analogWrite(led, intensite_led);}                 // 6 la led va s'éclairer au niveau de la valeur de intensite_led
  
//---------------------------MAIS SI LE BALLON EST FROID (moins de 40°C)-----------------------------------
       
  if(TEMPERATURE0 < 40 )        // 7 Si la température est inféfieur à 40°C
  {intensite_led = 250;}        // 7 on met la led à fond (pour chauffer le ballon d'eau chaude)
    
 //--------------------------- ET POUR AVOIR DE L'EAU A 55°C UNE FOIS PAr SEMAINE (pour eliminer la legionelose)-------     

 if (TEMPERATURE0 > 55)          // 7 Si le chauffe atteint naturellement 55°C (avec le photovoltaique), 
 {Compteur =0;}                  // 7 alors on remet le compteur a 0
       
 if (Compteur > 100000 )         // 7 si le compteur arrive à 100 000 (dans la réalité +/- 5 jours a vérifier =(+/- 432 000 secondes)
 {intensite_led = 250;}          //7 on met la led à fond (pour chauffer le ballon d'eau chaude)


 analogWrite(led, intensite_led);  // 6 allumé ou éteint, c'est le dernier qui parle qui pilote l'intensité de la led
  
  
  Serial.print(intensite_led);                //permet d'afficher sur l'ordinateur certaine valeur pertinantes pour la comprehention du programme
  Serial.print("   ");                        // cela permet aussi de voir ou en est le programme et de gérer les difficultés
  Serial.print(w_Chauffe_Eau);
  Serial.print("   w");
  Serial.print(TEMPERATURE0);
  Serial.println(" C");
  
  // --------------------ECRIRE SUR LCD ET POUR CHANGER LES VALEUR DuLCD AVEC UN BOUTON POUSSOIR------------------------------------------------

  lcd.backlight();                // 2 Allumer l'écran LCD
  lcd.clear();                    // 2 rafraichir l'écran LCD ( efface les données précédentes)
  
 impulsion = digitalRead(pin_bouton_changer_ecran);                //5 Demande à "impulsion" de prendre la valeur de "pin_bouton_changer_ecran"
  if (impulsion == 0) compteur_impulsion = compteur_impulsion + 1; //5 On incrémente le compteur d'impulsion s'il y a une impulsion sur le bouton
  
  if (compteur_impulsion == 1)                                     //5 Si le compteur d'impulsion est égale à 1 on affiche le premier écran    
        {
        //-----Ecran n°1:-------------------------------
        
        //1ere ligne, watt entrant et watts sortant----
        lcd.setCursor(0, 0);               // 4 positionner le curseur sur la ligne 1 et à gauche
        lcd.print("1 ");                   // 4 ecrire "In " pour comprendre qu'il s'agit de la consommation
        lcd.print(w_instantane_in,0);      // 4 puis écrire la valeur positive de la consommation au compteur EDF avec 2 chiffre derière la virgule 
        lcd.setCursor(9, 0);               // 4 rester sur la ligne 1 mais positionner le curseur sur le caractère 8
        lcd.print(w_instantane_out,0);      // 4 ecrire la valeur de w_instantane_out (c'est la surproduction) avec 2 chiffre derière la virgule 

        //2eme ligne, Watt solaire et watts chauffe eau
        lcd.setCursor(2, 1);              // 4 positionner le curseur sur la ligne 2 et à gauche
        lcd.print(w_Solaire,0);           // 4 puis écrire la valeur de la surproduction visible par EDF avec les compteurs electroniques (sans les virgules)
        lcd.setCursor(9, 1);              // 4 Rester sur la ligne 2 mais curseur position 8
        lcd.print(w_Chauffe_Eau,0);       // 4 puis écrire à la suite la valeur de la surproduction en Wh avec un chiffre derrière la virgule
        }

 else if (compteur_impulsion == 2) //5 Sinon s'il est égale à 2 on affiche l'écran numéro 2
        {
        // -----ECRAN N°2:-------------------------------
        //1ere ligne, kWh entrant et KkWh sortant--------
        lcd.setCursor(0, 0);            // 4 positionner le curseur sur la ligne 1 et à gauche
        lcd.print("2 ");                // 4 ecrire "2" pour comprendre que nous sommes sur l'écran n°2
        lcd.print(kwh_cumule_in,4);     // 4 puis écrire la valeur de la consommation en kWh avec 4 chiffres derrière la virgule
        lcd.setCursor(8, 0);            // 4 rester sur la ligne 1 mais positionner le curseur sur le caractère 9
        lcd.print(" ");                 // 4 ecrire " " pour séparer les deux valeur de cette ligne
        lcd.print(kwh_cumule_out,4);    // 4 ecrire la valeur des Watts cumulé en kWh, c'est la valeur de la surproduction

        //2eme ligne, KWh solaire et kWh chauffe eau
        lcd.setCursor(2, 1);              // 4 positionner le curseur sur la ligne 2 et position 2
        lcd.print(kwh_Solaire,4);         // 4 puis écrire la valeur de la production cumulée de mes panneaux solaire en kWh
        lcd.setCursor(8, 1);              // 4 rester sur la ligne 2 mais positionner le curseur en position 2
        lcd.print(" ");                   // 4 ecrire " " pour séparer les deux valeur de cette ligne
        lcd.print(kwh_Chauffe_Eau,4);     // 4 puis écrire la consomation cumulée de mon chauffe eau en Kwh
        }

else if (compteur_impulsion == 3)         //7 Sinon s'il est égale à 3 on affiche l'écran numéro 3
        {
        // -----ECRAN N°:-------------------------------
        //1ere ligne, Temperature du chauffe eau--------
        lcd.setCursor(0, 0);             // 7 positionner le curseur sur la ligne 1 et à gauche
        lcd.print("Chauffe eau ");        // 7 ecrire "Chauffe eau" pour comprendre que nous sommes sur l'écran n°2
        lcd.print(TEMPERATURE0,0);       // 7 puis écrire la valeur de temperature sans virgule
        lcd.print(" C");                 // 7 ecrire " C" pour Celcius
        

        //2eme ligne, Edition du compteur avant montée en température
        lcd.setCursor(0, 1);              // 7 positionner le curseur sur la ligne 2 et position 3
        lcd.print("Compteur "); 
        lcd.print(Compteur);              // 7 puis écrire la valeur du compteur
        }
        
 else if (compteur_impulsion == 4)       // 5 si on a appuié 4 fois sur le bouton poussoir,
        {
        compteur_impulsion = 1;          // 5 alors on remet le compteur à 0 et on retrouve l'écran n°1
        }

}                                 // FIN DU PROGRAMME
