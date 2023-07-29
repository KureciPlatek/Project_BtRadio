# Context and scope

In this part are explained the context in which the device is, on a business point of view (features, interactions, qualities required) as well as on a technical point of view, which means which technologies are used and where

# Business context

![System business context](./ARC42_contextesDiagram_business.svg "System business context")

|Entity|Purpose/Interaction|
|:-:|:-|
|User| Controls the radio. It can execute nextPrevious track, control volume level, search FM channel or save/search one, etc... User has also a second channel, which is from our system, to the User and it provides information about status, track played and so on...|
|FM sender| Is the FM sender antenna, it provides different station and use the standard FM network|
|Modern audio stream| Is the source of audio stream, it must respect constraints described in part I|
|Speaker| Is the speaker to output audio |