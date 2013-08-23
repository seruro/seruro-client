
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

#define TEXT_DOWNLOAD_INSTALL_IDENTITY "You may install the identity certificates. \
Please only install the certificates on a computer you trust and use regularly."

#define TEXT_INSTALL_IDENTITY "You will now install your Seruro Certificates. \
To install please enter the Encryption and Digital Identity unlock codes (sent via email or text message)."

//#define TEXT_DOWNLOAD_INSTALL_WARNING "Warning: \
//The Seruro admin has not approved your ceritifcate requests. \
//Please retry the download when your requests are approved."

#define TEXT_DOWNLOAD_INSTALL_WARNING "Could not download certificates, please retry."

#define TEXT_ASSIGN_IDENTITY " Email applications (like Outlook and Apple Mail) must be configured to use your Seruro certificates. \
Select your email account below, the use the 'Configure' button to associate your Seruro certificates to the application."

#define TEXT_SETUP_WELCOME "Welcome to Seruro! Let's take a moment and configure your computer.\
\n\
\n\
If this is your first time installing Seruro, this setup wizard will guide you through the following steps:\
\n\
\n\
\t 1. Connect to a Seruro server.\n\
\t 2. Install your Seruro certificates.\n\
\t 3. Configure your email application(s).\n\
\t 4. Download Seruro contacts.\n\
\n\
This setup may be canceled and restarted at a later time. After completing the setup you may \
add additional Seruro servers and accounts."

#endif