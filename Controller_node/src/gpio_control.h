#ifndef GPIO_CONTROL_H
#define GPIO_CONTROL_H

#include <stdint.h>

// Define the base address and size for GPIO on Raspberry Pi 4
#define GPIO_BASE 0xFE200000
#define GPIO_SIZE 0xB4        // The total size of GPIO control registers

// Function prototypes
int gpio_init(void);
void gpio_close(void);
void gpio_set_output(int pin);
void gpio_set_input(int pin);
void gpio_set_pullup(int pin);
void gpio_set_pulldown(int pin);
void gpio_clear_pull(int pin);
void gpio_write(int pin, int value);  // Set high (1) or low (0)
int gpio_read(int pin);               // Read pin state (1 = high, 0 = low)

#endif

//###%%%%%:      :===+++++*****************#################%%%%+%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@%%@@@@%@@@%%%%@@@@@@@@@@@@@@@@@@@@@@@@@%@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//###%%%%%.      .=======++++++*+*+********##################%%#*%%%@@@%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@%%%%@@%%@%@@@@@@@@@@@@@@@%@@@@@@@@@@@@@@@%@@@@@@@@@@@@@@@@@@@@@@@@@@
//####%%%%.       -======+++++++++++*******##################%####%%%%%@@@@@@@@@@@@@@@@@@%@@@@@@@@@@@%%%@@%@%%%@@@@@@@@@@@@@@%%%@@@@@@@@@@@%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//#####%%%:       ----====++++++++++********##**##################*%%%@@@@@%@@@@@@@@@@@@@@@@@@@@%%%%%%@@@@@@@%%%%@@%%%%@@@@@%%@@@@@@@@@@@@@@@@@@@%@@@@@@@@@@@@@@@@@@@@@@@@
//#####%%%:       .---====++++++++++++++*****#**################%*+%%%@@@%%@@@@@@@@@@@@@@@@@@@@%@%%@@@@@@@@@@@@@%%%%%%@@@@@@@@@@@@@@@@@@@@@@@@@@%@@@@@@@@@@@@@@@@@@@@@@@@@
//######%%:        ---======+++++++++++++*******#################+=%%@@@@@@@@@@@@@@@@@@@@@@@@@@%%%@@@@@@@@@@%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@%%@@@@@@@@@@@@@@@@@@@@@@@@@
//######%%:        :----=======++++++++++********################+-%%%%%@@@@@@@@@@@@@@@@@@@@@@%%%%@@@@@@@%%%%%%@@@@@@@@@@@@%%@%@@@@@@@@@@@@@@@@@%%@@@@@@@@@@@@@@@@@@@@@@@@
//######%%:        :-----==========+++++++*********##############+-%*#%@@@@@@@@%@@@@@@@@@@@@%#%#%%@@@@@%%%%%%#%%@@@@@@@%@@@@@%%%%@@@@@@@@@@%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//#######%.        .----====---======++++++*****************#####=.-%%%@@@@@@@@@@@@@@@@@@@@#*#%%%%%@@@%%@@@###@@@@@@@@%%%%%%%%%%%%@@@@@@@@%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//########:.        ----===-----=====++++++++****************#*#*.+%%%%@@@@@%@@@@@@@@@@@@#++*##%%%%%%%@@@@**%%%%%@@@@%%%%%%%%%%%%@@@@@@%@@%@@@@@@@@@@@@%@@@@@@@@@@@@@@@@@@
//########:     .   .---==------========+++++*+****++*********##-=%%%%@@%%%@@@@@@@%@@@%*++*###%%%@@%%%%%%%##%%%%@@@@@@@%%%%%@%%@@@@%@@@%@@%@@@@@@@@@@%%%%@@@@@@@@@@@@@@@@@
//########.     .    --===------=========+++++++**++++********##:=%%%@%##%@@@@@@%%@@%*+*#%%#****###%%#%@@%*%%@%%@@%@%%%@%%%@@@@@@@@@@@%%@@%%%@@@@@@%%%%%%@@@@@@@@@@@@@@@@@
//########.     ..   .-==------==========++++++++++++++******####--###=*%@@@@@@%@@@#*+%@%##***+++++***%%@%+*%@%%@@%%%%%@@%@@@@@@@%@@@@@%@@@@@@@@@@@@@@@%@@@@@@%@@@@@@@@@@@
//########.      .    :=------==========+++++++++++++++*******####*-:.-%@@@@@@%@@%#**%@%#**********%##%@@*+*##%%@@%%%%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@%@@@@@@@@@@@@@
//########:      .    .=-----=============+++++++++++++*******#####* .%@@@%%@@@%*##**@@%*********##%%@%#*+++*#+#@%%%%%%%@@@@@%@@@@@@@@@@@@%%%@@%@@@@%%@@@@@@@@@%@@@@@@@@@@
//########:            :-----=============+++++++++++++++++***#####*:=@@%%@@@@#**%#*#@@#*********####**************#%%@%%%%%@@%@@@@@@%%@@@@%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//########:             :----============+++++++++++++++==+**########:%%%@@@%***##**%@%********###%%#**************#%%**%%#%%%@@@%@@%%%@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//########:              :---=============++++++++++++==++++*########-#%@@%+**#+#***@@%#*#*####%%%%%%##**#**#######*#%%##%%%@%%%##@@@%@@@@@@@@@@@@@@@@@@%@@@@@@@@@@@@@@@@@
//*#######:.              --================++++++++===+++****########-#%%--+*-****%@%%###%%%%%%%%%%###******######***#*+#%#%@%%*%@%#%@@@@@@@@@@@@@@@@@@@@@@@@@%@@@@@@@@@@
//**######:.              .-==================+++=--=++++*****############*..:-#**#@@@##%%##*###%%%**#********#####**##*+*#%%%@%%@@@%@%@@@@@%%@%%@@@%%@@@@@@@%%%%@@@@@@@@@
//***#####:..              .====================:..=++++******##############- =**%%@@%#%#+**###%#%**#**************###***#%%@@%%%@@@%@@@@@@@%%%%#%@@@@%@@@@@@@@@@@@@@@@@@@
//***#####:..               :==================:.-=+++*********###############::*%@@%%%#.-+**##:#********+++****########%%%%%%%%%%%@@@@@@@@#%@%#%%#@@@@@@@@@@%%@@@@@@@@@@@
//*****###....               -======--=======-:-=+++++*********##############%#:*@@@@%%@%-**%+.=#**#*****+++****#########%%%%@%%@%@@%%@@@@@@@%##%%%@@@@@@@@@@@@@@@@@@@@@@@
//*****###....                -=====----===----=++++++**********###############=-%%+:*%%@%*@%*+%#%%***********##########%%@@@@%%%###%@@@@@@@%##%#%%%%%@@@@@@%%@@%@@@@@@@@@
//*******#...                  -=====-----:---==++++++**********################*::=:**#%%%%%%*#%#*********###########%%%%@@@%%%%##%%%%%@@@%%%####%%@@%%@@@@@%%%%@@@@@@@@@
//*******#.....                 -====--::::--===+++++++**********##################%- :%%###%%%%#++******##########%%%%%%@@%%%%%####%%%%@@@@@%%#%%%@@@@@@@@@%%###%@@@@@@@@
//********.......               .===--:.::---====+=++++**********#*#################*.-%#####*#%#*+++***#########%###%%%@@%%%@#%#####%%%%@@@@%%%%%%%#%%@@@@@@%##*%@@@@@@@@
//********   .....               ---:::::-----=====++++**************###############-.%%####***#############%%%%###%%%@@@%#%%########%%%%%%@%%%%%%#%%%%@@@@@@%##*%@@@@@@@@
//********      ..                :.:::--------====++++**********#*#*##############- +%%#####***##*+*#####*#%%###%%%%@@%#*%%#########%%%%%%@@%%%%%#%%%@@@@@@%%##*%@@@@@@@@
//********                         ::---------=====++++**********#****######*#####+ .%%%####****#++++++*#%#+#####%%@@%#*#%%#*##########%%%%@@%%%%%%@%%@@@@@@%%#**%@@@@@@@@
//********                         :----------======+++***********************####: =%%%###***+*+++**+*#%*+#####%%@@%##%%%#*############%#%%@@%%%%%%%@@@@@@@%%##*%@@@@@@@@
//********                         :---------======+++++********+==******#########= -%%%###**+++++***###%######%@@@#*%%%##**#############%%%@@@@%%%%@@@@@@@@%%%#*%@@@@@@@@
//********                         .--------=======++++++++++==++******###########* .*%####**+**+**###%%#####%%@@%*#%%%#**#############%%%%%@@@@%%%%@@@@@@@@%%%#*%@@@@@@@@
//********                          --==----=======+++++==++++*********############= +%%##********##%%%%%###%@@@#*%%%%#***#############%%%%%@@@%@@@%@@@@@@@@%%%#*%@@@@@@@@
//********                          --==----==========++++++++********##############.-#%##*******###%%%%%%%@@@%++%%%%****#*############%%%%%@@@@@@@@@@@@@@@@%%%#*%@@@@@@@@
//********                          :==-----:--::-===++++++++++*******#############%= ####******###%%%%%%@@@%#+*%%%%*****#***#**#######%%%%%%%%@@@@@@@@@@@@@%%%##%@@@@@@@@
//********            ..            :---::....:--==++++++++++++**####**#############* +###****###=##*%@%%%%#*+*%%%%*+***##*******######%%%%%%%%@@@@@@@@@@@@%%%%%#%@@@@@@@@
//********            ...           .-:...::--===++++++++++++++**##*#*##############%:-*###**##*  -:=#%@@%#++#%##%*++**#*********#####%%%%%%%%@%@@@@@@@@@@%%%%%##%@@@@@@@@
//********            ..             :-------===+++++++++++++++**#*##***###########%%:-###**###-*= =#%%##+=*%%#%%+=++*####**+****#####%%@%@%#%@@@@@@@@@@@@@%%%%##%@@@@@@@@
//********                          .==------===++==+++++++++++**#******############* +######%=+%= =++=-+#@%##%#=-++*####*******#######%%%%%%@@@@@@@@@@@@@%%%%%##%@@@@@@@@
//********                          :=--------======+++++***+++==+*+****###########%-:#######==%%*. :=#%%##*=.::-++*####********######%%%%%%%%@@@@@@@@@@@@@%%%%##%@@@@@@@@
//********                          -==--------=====+++++---*#**-===*****###########.+%#####+=%%%%%%%@%%%%:  ..:++#####*********########%%#%%%@@@@@@@@@@@@%%%%%##%@@@@@@@@
//********             .            ==---------===++++=:-+*#%%%###**+*****########* -%%#####+#%%%%%%%%%%%*+  .-=+#####******##########%%%%%@@@@@@@@@@@@@@@%%%%%##@@@@@@@@@
//********             .           :==--------=====.:-=+##%#%%%##*+-=*******######: #%%%%#*=%%%%%%%%%%#-**++  -*####++******#########%%%%%%%%@@@@@@@%%%@%%%%@%%%%@@@@@@@@@
//********             .           -====-------==: .=##%%#%%%%%#*+==+*******#####- *%%%%#**%%%%%%%%%%%:-++**###%%%+ -+******#######%%%%%%%%@@@@@@@#-::-+**#######@@@@@@@@@
//********                        :=====-------:. .*#%*#%%%%%#***+==*********####:.*%%#*=#%#%%%%%%%%%+ -+*###%%%#%%+.++*****######%%%%%%%@@@@@@@%+.:::=--+*+++++*%@@@@@@@@
//*******#                        -========-:..--+##*#%%%#####*===+***********####+---=*###%#%%%%%%%#:.+**%#%%##%%%%::**#**######%#%%%%%%%@@@@@@*:::::---=+****++%@@@@@@@@
//******##                       -=======-:. :=+***%%%%####*+==+***************#############%%%%%%%%= :*##%%%##%%%%%= =*#######%%%%%%%%%%%@@@@@#--:...:==-=*****+%@@@@@%%%
//*****###                      .--=-==-::--:=#**%%%%%%#**+=-+**************################%%%%%%%*::*##%%%*%%%%%%%* -*%###%%%%%%%%%%%%@@@@@@#=-.  .---+*#******%@@@@%%%%
//*****###                     .------..-=++*++%%%##**++=-=*++*************#################%%%%%%#:--##%%%#%%%%%%%%% .*%%###%%%%%%%%%%%@@@@@@#:.  :-+=++*+=++***%@@%%%%%%
//****####                    .:-----  :=+*==#%%%#**+=--=+++**************#################%%%%%*:==*=%%%##%%%%%%%%%%. +#%##%%%%%%%%%%%@@@@@@#:..:--==-====-=++**%@%%%%%%%
//***#####                   .:---::  .=*+:*%%%#**+=+++++++++**++*********#################%%%%= -*+*+%%##%%%%%%%%%%%  :*%%#%%%%%%%%%%@@@@@@%+...:.--==--:-=*****%%%%%%%%%
//**######                  .---: .   :=:+%%%#**++=-+++++++++**+**********#################%%#.:-=##%#%#%%%%%%%%%%%%*  ..*%%%%%%%%%%%@@@@@@%*-.       ..:::.-++**%%%%%%%%%
//########                .:::::     -..*%#*+==---=++++++++**+********#****################%* -=+*%%%%*%%%%%%%%%%%%%+    .+%%%%@%@@@@@@@@@@%=.  ...  .:----::---=#%%%%%%%%
//########              :-:-==:   ..:.+%%#+==----=+++++++*++*********######################+ :++#%%%%*#%%%%%%%%%####*.   -=#%%@@@@@@@@@@@@@#-:.   .----=+**++=---#%%%%%%%%
//########:    :=++   --+====:   ::.-#%*+===--=++++++++*++++********######################=.:+##%%%%*%%%%%%%#%####%@@*..-=+%@@@@@@@@@@@@@@@*.   .:-+*##*==++*##*+#%%%%%%%%
//########=-..:-----::=++====   :-:+%*====--:-++++++++**+++++*******#####################*..+##%%%%+#%%%%%#####*#%%@@%****%%@@@@@@@@@@@@@@@%=  :+#+-::::::----===#%%%%%%%%
//########**#######*+*+=+++==::==:*%%*+=-::.:=++++++++*+****+*******#####################-.:*#%%%%+#%%%%###%####*#%%%@%@%%@@@@@@@@@@@@@@@@@%-...   ..-===---:---=#%%%%%%%%
//########====+++++-=*++=-  .--=-#*+=-:. .:-===+++***+*************#####################.=--#%%%%+*%%%%######*++%%%%%%%@%@@@@@@@@@@@@@@@@#=      :+********#*****#%%%%%%%%
//########+++*####*==+=+=:. :==-##*+--::-=======+++**************###################%##+ +*##%%%++%%%%######*+=*%%%%%%%%@%@@@@@@@@@@%%%#-    .-***+=-==++++****###%%%%%%%%
//########==:-=+***+=*==-..-+=+#+==:..:=+++++=====+************####################%%##::*##%%%++%%%########*-#%%%%%%%@@@@@@%%%%%%%@@%=    :+*#####%%%%%%%%%%##*+#%%%%%%%%
//########+=::+*###*++=-..:+-+#+-::::-+++++++++++**+=+********######################%%=.-**%%%*=%%%##*#####*++%%%%%%%@@@@@@@%%%%%%@%+   .=**%%%%%%%%%%%%%%%%%%%##%%%%%%%%%
//%%%%%%%%+=--=###%#++=: :+-*+-===+++++++++++*++******++******###################%%%%*::+*#%%*.#%%###****#+==%%%%%%%%%%@@@@@%%@@%%@:  :==*#*#%%%%%%%%%%%%%%%%####%%%%%%%%%
//%%%%%%%%*+--+##%%*:=. .=-**--.::====++++***************++****###############*=:::--:..**#%*:*%%###***++==+#%%%%%%%%%%@@@@@@@%%%@* .-++++#%%%%%%%%%%%%%%%%%#####%%%%%%%%%
//%%%%%%%%*+--+####*=.  -:+-. ..-=+==+++++**************#*******############*:     ..-:-++##-+%%##****+==+%%%%%%%%%%@%@@%@@@@%%@%%*:-==+#%%%%%%%%%%%%%%%%%%######%%%%%%%%%
//%%%%%%%%*+-:+####=:  --+:-=+++++++++++++**************######***########*++.  .=**#==+-+##--#%####***+==#%%%%%%%%%%#=::+%@@@%%%*====+*#%%%%%%%%%%%%%%%%%######**#%%%%%%%%
//%%%%%%%%:...:=+*%+=.:-+-..:=+++++++++++********###****#######**#*###%%#+: =+.=*##+=*++=#=:#%#*###*+==*%%%%%%%#   -   .+=#***=---=+###%%%%%%%%%%%%%%%%#######***%%%%%%%%%
//%%%%%%%%%%@@@@@@@%%::=-++-:##+++++++++********#####*#############*-+%#**#.-***####**+*=#-*#####**=:.:=++=*#*#=   = .:==   .-:+####%%%%@@%%%%%%%%%%%#########***%%%%%%%@@
//%%%%%%%%   ::++*+==-=+=:.:*+**++++++++********#####*############*-*%%=  =.=*#%%%####****+*%#**#*###+=+*##*- .:-::.::.:-  .=*:*%%%%%%%%@@%%%%%%%%%##########****%%%%@@@@@
//%%%%%%%%++#@@@@@@@@@@@@%#=-+***+++++++********#################*=#%%%+-*:=#*++#%##%##*#-*#%###%===+*##*#####+:.-===-===..-=.   :=+%@@@@%%%%%%%%%######*********%@@@@@@@@
//%%%%%%%%+=+*#%%%@@%@@@@@%#@****+++++****##*#################*=  .*%@%%:.*+*#: .#%%#%##:*###%%%+-:##-%*%%*##%%%#--=+*#%%##%%-   :+*#@@%%%%%%%%%######**********+%@@@@@@@@
//%%%%%%%%@@@@@@@@@@@@@@@@%#%%****+*****#####################:  .= :-+:====+%%*+=#%=#%#++*##%%#+*#++*+++%%#%%%%%%%%###%%%%@@%*-++*#+#%%%%%%%%%%#####*********+++*%@@@@@@@@
//%%%%%%%%@@@@@@@@@%%%%@@@@@@%#+##******####################.  -##. -=+*=-*%%%%%#-= *##+*#%@%%#*+-==++*%%#+=%%%%%%%%%%%%%%%%%%%#***=  +*#%%%%%%######*****+++++++%@@@@@@@@
//%%%%%%%%@@@@@@%%%%@@@@@@@@@@@@%%@@%***###################*  .*#.   :=+##%%%%%#%*=:*##**%@@%#+==++=+*##%%=+#%%%%%%%%%%%%%%%%%%%%%-..:+###%%%%%######****++++++++%@@@@@@@@
//%%%%%%%%@@@@@@@@%%@%*-::+%@@@@@%**@@#####################+  -+.   :=*##%%%%%####=%+#**%@@@##+-=+*##*###%@%%@%%%%%%%%%%@%%%%%%%%%=**+####%%######*****+++=-####*%@@@@@@@@
//%%%%%%%%@%**+=--==*#= .=%@@@@@@@%=-%@###################*   =:   :=+*##%##%%%####+##*#@@@%+--==+**%%%%#%@%#%%%%%%#%%%%%%%%%%%%%%%#%%#**%%%%#####***+++*=.*##*+*%@@@@@@@@
//%%%%%%%%@@@@@@@@@@@%%#%@@@@@@@@@@@#*@%*################=  -..:   -=+##%%%##%%%%##@%##%@@@*=--:-+*#%%%%%%@+#%%%%%%#%%%%%%%%%%%%%%%%%%#+.-+*%%%%#**+++++= +##****%@@@@@@@@
//%%%%%%%%@@@@@@@@@@@@@@@@@@@@@@@@@@@#%%#*#############+. .+**++.  -=**##%%%##%@@@@@###@@@#+=--=+**#%%%%%%#*%%%%%%#%%%%%%%%%%%%%%##%%###+=***%%#*****++=:-##****#%@@@@@@@@
//@@%%%%%%@@@@@@@@@@@@@@@@@@@@%%%@@%@@@@%*#########***=   ==+#%*+  :+++*#%%%%%#%@@@@#%%@@%*---=+*##%%%%%%%%@%%%%%%#%%%%%%%%%%%%%%#%%%%*#%##*#%##**++++=-*########%@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@%%%%%@@@@@@%@==----======-   .***#-%#=..-=+**#%%%%#%@@%%%%@@%#=::-==*#%%%%%%@@@%%%%%%##%%%%%%%%%%%%###%%%%%#*++*%%#*++++++=+#%#####%#%@@@@@@@@
//@@@@@@@@@@@@@@@%%%%%@@@@%%%%%%@@@%%@@%%-===========-   :#****%@#+ :=++*#%%%%#%@@@#%%@##+==+++**#%%%%%%@@%%@%%%###%%%%%%%##%###########*+:-++*********%%%##%%%%%@@@@@@@@@
//@@@@@@@@%@@@@@%%%%%%%%%%%%%%%%@@@%%%%%*-==========--   -#+++*#%%*- :-=**#%%%%%@@@#%@%**==++++*#%%%%%%@@@%%%%###%%%%%%%%%%%########%%%***=+***###%%###%%%#%%%%%%@@@@@@@@@
//@@@@@@@@%%%@@@%%%%%%%%%%%%%%%@@@@%#%%%---=========---  :*=+*%%*-=* .==**#%%%%%@@@%%@#*#%#***##%%%%%%@%%%#####%%%%%%%%%%%#######%%#%%%#+###%%%%%%######%%%%%%%@%@@@@@@@@@
