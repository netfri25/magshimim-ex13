mod message;
mod chat;
mod server;
use server::Server;

fn main() {
    Server::bind("localhost:6969").unwrap().serve()
}
