use std::collections::BTreeSet;
use std::fs::File;
use std::io::{self, Read, Write};
use std::net::{TcpListener, TcpStream, ToSocketAddrs};
use std::sync::mpsc;
use std::sync::{Arc, RwLock};

use crate::chat::ChatMessage;
use crate::message::Message;

pub struct Server {
    listener: TcpListener,
    connected_users: RwLock<BTreeSet<String>>,
}

impl Server {
    pub fn bind(port: impl ToSocketAddrs) -> io::Result<Self> {
        Ok(Server {
            listener: TcpListener::bind(port)?,
            connected_users: Default::default(),
        })
    }

    pub fn serve(self) {
        let me = Arc::new(self);
        let (send, recv) = mpsc::channel();
        std::thread::spawn(move || {
            let this = Arc::downgrade(&me);
            while let Some(this) = this.upgrade() {
                this.accept_client(send.clone()).unwrap_or_else(|e| {
                    eprintln!("[ERROR] unable to accept client: {e}");
                });
            }
        });

        // handle messages queue
        // TODO: when should I stop?
        loop {
            for msg in recv.iter() {
                let filename = get_names(&msg) + ".txt";
                let mut file = match File::options()
                    .append(true)
                    .create_new(true)
                    .open(&filename)
                {
                    Ok(file) => file,
                    Err(e) => {
                        eprintln!("[ERROR] unable to open file {filename}: {e}");
                        continue;
                    }
                };

                if let Err(e) = file.write_all(msg.to_string().as_bytes()) {
                    eprintln!("[ERROR] unable to write to file {filename}: {e}");
                    continue;
                }
            }
        }
    }

    pub fn accept_client(self: Arc<Self>, channel: mpsc::Sender<ChatMessage>) -> io::Result<()> {
        let (conn, addr) = self.listener.accept()?;
        eprintln!("[INFO] accepted {addr}");
        std::thread::spawn(move || {
            self.handle_client(Connection(conn), channel)
                .unwrap_or_else(|e| {
                    eprintln!("[ERROR] unable to handle client: {e}");
                });
        });
        Ok(())
    }

    pub fn handle_client(
        &self,
        mut client: impl Read + Write,
        channel: mpsc::Sender<ChatMessage>,
    ) -> Result<(), io::Error> {
        let Message::ClientLogin { username } = Message::read(&mut client)? else {
            return Err(io::Error::other(
                "First message is expected to be a ClientLogin",
            ));
        };

        // this weird chunk of block emulates the `defer` keyword in Golang
        struct Dropper<F: FnMut()>(F);
        impl<F> Drop for Dropper<F>
        where
            F: FnMut(),
        {
            fn drop(&mut self) {
                (self.0)()
            }
        }
        let _dropper = Dropper(|| match self.connected_users.write() {
            Err(e) => eprintln!("[ERROR] unable to drop client handler: {e}"),
            Ok(mut users) => {
                users.remove(&username);
                eprintln!("[INFO] {username} disconnected");
            }
        });

        self.connected_users
            .write()
            .map_err(|e| io::Error::other(e.to_string()))?
            .insert(username.clone());

        client.write_all(
            Message::ServerUpdate {
                chat_content: String::new(),
                recipient: username.clone(),
                connected_users: self
                    .connected_users
                    .read()
                    .map_err(|e| io::Error::other(e.to_string()))?
                    .clone(),
            }
            .to_string()
            .as_bytes(),
        )?;

        loop {
            let msg = Message::read(&mut client)?;
            match msg {
                Message::ClientUpdate { recipient, message } => {
                    let msg = ChatMessage::new(&username, recipient, message);
                    if msg.recipient.is_empty() || msg.data.is_empty() {
                        let filename = get_names(&msg) + ".txt";
                        let chat_content = std::fs::read_to_string(filename).unwrap_or_default();

                        client.write_all(
                            Message::ServerUpdate {
                                chat_content,
                                recipient: msg.recipient,
                                connected_users: self
                                    .connected_users
                                    .read()
                                    .map_err(|e| io::Error::other(e.to_string()))?
                                    .clone(),
                            }
                            .to_string()
                            .as_bytes(),
                        )?;

                        continue;
                    }

                    eprintln!("[INFO] adding {msg:?} to the queue");

                    channel.send(msg.clone()).map_err(io::Error::other)?;

                    let recipient = msg.recipient.clone();
                    client.write_all(
                        Message::ServerUpdate {
                            chat_content: msg.to_string(),
                            recipient,
                            connected_users: self
                                .connected_users
                                .read()
                                .map_err(|e| io::Error::other(e.to_string()))?
                                .clone(),
                        }
                        .to_string()
                        .as_bytes(),
                    )?;
                }

                Message::ClientDisconnected | Message::ClientFinish | Message::ClientExit => break,
                Message::ClientLogin { .. } | Message::ServerUpdate { .. } => {
                    return Err(io::Error::other("Invalid message from client: {:m?}"));
                }
            }
        }

        Ok(())
    }
}

fn get_names(msg: &ChatMessage) -> String {
    let mut output = String::with_capacity(msg.author.len() + 1 + msg.recipient.len());

    if msg.author > msg.recipient {
        output.push_str(&msg.recipient);
        output.push('&');
        output.push_str(&msg.author);
    } else {
        output.push_str(&msg.author);
        output.push('&');
        output.push_str(&msg.recipient);
    }

    output
}

// used for easier logging
struct Connection(TcpStream);

impl Read for Connection {
    fn read(&mut self, buf: &mut [u8]) -> io::Result<usize> {
        let read = self.0.read(buf)?;
        println!(
            "[INFO] reading: {}",
            String::from_utf8(buf.to_vec()).unwrap()
        );
        Ok(read)
    }
}

impl Write for Connection {
    fn write(&mut self, buf: &[u8]) -> io::Result<usize> {
        println!(
            "[INFO] writing: {}",
            String::from_utf8(buf.to_vec()).unwrap()
        );
        self.0.write(buf)
    }

    fn flush(&mut self) -> io::Result<()> {
        self.0.flush()
    }
}
