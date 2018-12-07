Сервис для создания и участия в голосованиях
Позволяет создать новое голосование и получить ссылку на него(для рассылки участникам) и на его результаты.
Позволяет проголосовать и увидеть результаты голосования.
Сервис будет стараться соответствовать REST-архитектуре.

1) создать новое голосование
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
      "type": "application/x-www-form-urlencoded",
      "fields": [
        { "name": optionID, "type": "radio", "title": "" },
        { "name": optionID, "type": "radio", "title": "" },
        ...
        { "name": optionID, "type": "radio", "title": "" }
      ]
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

Возможно, вместо передачи вариантов ответа в properties сделаю список вариантов отдельным гипермедиа (polls/pollID/options)
и вынесу список вариантов в отдельную sub-entity.

    