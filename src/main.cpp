#include "pch.hh"

#ifndef ASIO_STANDALONE
namespace asio = boost::asio;
#endif
using json = nlohmann::json;
namespace dpp = discordpp;

std::string getToken();

std::istream &safeGetline(std::istream &is, std::string &t);

void filter(std::string &target, const std::string &pattern);

int main() {
  dpp::log::filter = dpp::log::info;
  dpp::log::out = &std::cerr;

  std::cout << "Starting bot...\n\n";

  std::string token = getToken();

  auto bot = std::make_shared<DppBot>();
  bot->debugUnhandled = true;
  bot->intents = dpp::intents::NONE | dpp::intents::GUILD_MESSAGES;
  dpp::User self;
  bot->handlers.insert(
      {"READY", [&self](dpp::ReadyEvent ready) { self = *ready.user; }});

  bot->prefix = "?";
  bot->respond("ping", "Pong !");
  bot->respond("test", [&bot](dpp::MessageCreateEvent msg) {
    std::cout << "Running Tests" << std::endl;

    bot->createMessage()
        ->channel_id(*msg.channel_id)
        ->content("tEsTs good")
        ->run();

    std::cout << "Done" << std::endl;
  });

  // Create Asio context, this handles async stuff.
  auto aioc = std::make_shared<asio::io_context>();

  // Set the bot up
  bot->initBot(9, token, aioc);

  // Run the bot!
  bot->run();

  return 0;
}

std::string getToken() {
  std::string token;
  char const *env = std::getenv("BOT_TOKEN");
  if (env != nullptr) {
    token = std::string(env);
  } else {
    std::ifstream tokenFile("token.dat");
    if (!tokenFile) {
      return "";
    }
    safeGetline(tokenFile, token);
    tokenFile.close();
  }
  return token;
}

std::istream &safeGetline(std::istream &is, std::string &t) {
  t.clear();
  std::istream::sentry se(is, true);
  std::streambuf *sb = is.rdbuf();

  for (;;) {
    int c = sb->sbumpc();
    switch (c) {
    case '\n':
      return is;
    case '\r':
      if (sb->sgetc() == '\n') {
        sb->sbumpc();
      }
      return is;
    case std::streambuf::traits_type::eof():
      // Also handle the case when the last line has no line ending
      if (t.empty()) {
        is.setstate(std::ios::eofbit);
      }
      return is;
    default:
      t += (char)c;
    }
  }
}
