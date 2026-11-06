# RoboCab

## Auto start
After you change the code. The service keeps running the old version until you run sudo systemctl restart robocab-camera, or reboot. This is the one people usually forget.
When you want to run the script by hand, e.g. with TUNE = True. Stop the service first so you don’t get “camera busy” and “port already in use”.
When something stops working. systemctl status and journalctl show you why.

Services running on the pi:

robocab-camera
roboca-server

If multiple users are connected to the server messages from anyone but the first connection will not be sent. Please close old connections or restart the pi with "systemctl restart robocab-server"

To listen to logs on the pi while sshd in use:
"journalctl -u robocab-[server/camera] -f"

