Сервис для создания и участия в голосованиях
Позволяет создать новое голосование и получить ссылку на него(для рассылки участникам) и на его результаты.
Позволяет проголосовать и увидеть результаты голосования.
Сервис будет стараться соответствовать REST-архитектуре.

1) создать новое голосование - (не реализовано по согласованию, работа с предзаполненной базой)
POST /polls
content-type application/json
{
    "Title" : "",
    "Options" : [ "", "", ... ]
}
Response:
201 Created
Location: /polls/pollID
content-type application/vnd.siren+json
{
    "class": [ "" ]
    {
        "links" : [
            { "rel": [ "self" ], "href": "/polls/pollID" },
            { "rel": [ "related" ], "href": "/polls/pollID/results" }
        }
    }
}

2) получить определенное голосование
GET /polls/pollID
Response:
200 OK
content-type application/vnd.siren+json
Если пользователь не прогосовал, в json-ответе будет поле
"actions": [
    {
      "name": "vote",
      "title": "poll text",
      "method": "POST",
      "href": "/polls/pollID/results",
      "type": "vnd.siren+json",
      {}
    }
  ],
иначе
   "properties" : 
   { текст голосования и результаты},
   "links" : [
        { "rel": [ "self" ], "href": "/polls/pollID/results" }
    ]
   
3) проголосовать (через "vote" action)
Response:
200 OK
content-type application/vnd.siren+json
{
    "class": ["poll"],
    "links" : [
        { "rel": [ "self" ], "href": "/polls/pollID/results" }
    ]
}
    

4) посмотреть результаты
GET /polls/pollID/results
Response:
200 OK
content-type application/vnd.siren+json
{
    "class": ["poll"],
    "properties":
    { текст голосования и результаты}
    "links" : [
        { "rel": [ "self" ], "href": "/polls/pollID/results" }
    ]
}


    
