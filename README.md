## This is an example project for NewtonOS tasks and memory management ##

Notes:

* Deleting a TUAsyncMessage which was sent prior to it being received will cause a -10015 error (invalid object ID). To investigate further: either use RPC to delete the message once it has been received, check the usage of kMsgType_Collected... flags, or use Receive which uses TUAsyncMessage
* Resending a TUAsyncMessage will cause a -10017 error (message already posted) if it has not been received in the meantime
* Receive with a TUAsyncMessage paramter is asynchronous
* When sending a TUAsyncMessage, the content parameter cannot be deleted or go out of scope before the message is received
* TUPort::Send with a TUAsyncMessage parameter is a convenience function which copies the data to be sent into the fMsg instance variable of the message