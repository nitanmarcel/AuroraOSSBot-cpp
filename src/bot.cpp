#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <tgbot/tgbot.h>
#include <curl/curl.h>

#include "config.h"


bool dispenser_check() {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://www.auroraoss.in:8080"); // "https://www.auroraoss.in:8080"
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        long http_code = 0;

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        return http_code == 200 && res != CURLE_ABORTED_BY_CALLBACK;
    }
    return 0;
}

int main() {
    TgBot::Bot bot(BotConf::token);

    // Setup Keyboard


    // Start Command
    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Up and running");
    });

    // Status Command

    bot.getEvents().onCommand("status", [&bot](TgBot::Message::Ptr message) {
        if (dispenser_check()) {
            bot.getApi().sendMessage(message->chat->id, "The Token Dispenser is up!");
        } else {
            bot.getApi().sendMessage(message->chat->id,
                                     "The Token Dispenser is down! It will be fixed as soon as possible!");
        }
    });

    // Suggestions
    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {

        TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);
        std::vector<TgBot::InlineKeyboardButton::Ptr> row0;
        TgBot::InlineKeyboardButton::Ptr approveButton(new TgBot::InlineKeyboardButton);
        TgBot::InlineKeyboardButton::Ptr rejectButton(new TgBot::InlineKeyboardButton);
        approveButton->text = "Approve!";
        rejectButton->text = "Reject!";
        approveButton->callbackData = "app";
        rejectButton->callbackData = "rej";
        row0.push_back(approveButton);
        row0.push_back(rejectButton);
        keyboard->inlineKeyboard.push_back(row0);

         std::string response;
         bool send_message = false;

         if (StringTools::startsWith(message->text, "/suggestion")) {
             response = "This suggestion is awaiting admin approval!";
             send_message = true;
         }
         if (StringTools::startsWith(message->text, "/bug")) {
             response = "This bug report is awaiting admin approval!";
             send_message = false;
         }

         if (send_message) {
             if (message->replyToMessage) {
                 bot.getApi().sendMessage(message->chat->id, response, false,
                                          message->replyToMessage->messageId,
                                          keyboard);
             } else {
                 bot.getApi().sendMessage(message->chat->id, response, false,
                                          message->messageId, keyboard);
             }
         }
     });

    // Buttons check

    bot.getEvents().onCallbackQuery([&bot](TgBot::CallbackQuery::Ptr query){

        TgBot::InlineKeyboardMarkup::Ptr empty_keyboard(new TgBot::InlineKeyboardMarkup);

        if (StringTools::startsWith(query->data, "app")) {
            // bot.getApi().forwardMessage(BotConf::channel_id, query->message->chat->id, query->message->replyToMessage->messageId);
            bot.getApi().editMessageText("Suggestion approved by " + query->from->firstName, query->message->chat->id, query->message->messageId);
        }
        if (StringTools::startsWith(query->data, "rej")) {

        }
    });

    // Miscs

    signal(SIGINT, [](int s) {
        printf("SIGINT got\n");
        exit(0);
    });

    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        bot.getApi().deleteWebhook();

        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            longPoll.start();
        }
    } catch (std::exception &e) {
        printf("error: %s\n", e.what());
    }

    return 0;
}
