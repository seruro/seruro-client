
#ifndef H_SeruroLanguage
#define H_SeruroLanguage

#define TEXT_ACCOUNT_LOGIN "Please enter your account information to log into the Seruro Server: "

#define TEXT_DECRYPT_METHOD_EMAIL "Please enter the 'unlock code' you received via email."
#define TEXT_DECRYPT_METHOD_SMS "Please enter the 'unlock code' you received as a text message."

#define TEXT_DECRYPT_EXPLAINATION "This is NOT your Seruro Account 'password'. \
This 'unlock code' will only be used to setup your personal certificates and does \
not need to be remembered or saved: "

#define TEXT_ADD_SERVER "Please enter the following details about the \
Seruro Server."

#define TEXT_ADD_ADDRESS "Please enter your email address and Seruro passowrd used to log into your Seruro Server. \
WARNING: Do NOT enter the password used to log into your email account!"

#define TEXT_REMOVE_SERVER "This will remove the Seruro Server '%s'. \
You may optionally uninstall the associated certificate authority (CA), \
certificates downloaded from '%s', and your '%s' account identities."

#define TEXT_REMOVE_ADDRESS "This will remove the account '%s' on the server '%s' from this machine. \
You may optionally uninstall the associated identity."

#define TEXT_INSTALL_IDENTITY "You may install the identity certificates. \
Please only install the certificates on a computer you trust and use regularly."

#define TEXT_ASSIGN_IDENTITY "Please assign your identity to an email account configured on your computer. \
Email can only be secured from an assigned account. \
Find the account name and email program below. Then use the 'Assign' button to associate the installed identity to the account. \
Note: some email programs will automatically assign the identity."

#define TEXT_SETUP_WELCOME "Welcome to Seruro! Let's take a moment and configure your client.\
\n\
\n\
If this is your first time installing the Seruro Client, please pay attention as some decisions may \
affect your privacy. This initial setup wizard will guide you through:\
\n\
\n\
\t 1. Connecting to your Seruro Server.\n\
\t 2. Configuring your account and downloading your secure identity.\n\
\t 3. Automatic setup of your email applications.\n\
\t 4. Retreival and installation of contact identities.\n\
\n\
This setup may be canceled and restarted at a later time. After completing the setup you may \
add additional servers and accounts, as well as change any setting."

#endif