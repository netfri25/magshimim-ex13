use std::collections::BTreeSet;
use std::io::Read;
use std::{fmt, io};

#[repr(u8)]
#[derive(Debug, Clone)]
pub enum Message {
    ClientDisconnected, // when the client disconnects

    ClientLogin {
        username: String,
    } = 200,

    ClientUpdate {
        recipient: String,
        message: String,
    } = 204,

    ClientFinish = 207,
    ClientExit = 208,

    ServerUpdate {
        chat_content: String,
        recipient: String,
        connected_users: BTreeSet<String>,
    } = 101,
}

impl Message {
    pub fn read(mut reader: impl Read) -> io::Result<Self> {
        let mut buf = [0u8; 3];
        let read = reader.read(&mut buf)?;

        if read != buf.len() {
            return Ok(Self::ClientDisconnected)
        }

        Ok(match buf.as_slice() {
            b"200" => Self::decode_client_login(reader)?,
            b"204" => Self::decode_client_update(reader)?,
            b"207" => Self::ClientFinish,
            b"208" => Self::ClientExit,
            b"101" => Self::decode_server_update(reader)?,
            _ => return Err(io::Error::other("Invalid code for message")),
        })
    }

    fn decode_client_login(reader: impl Read) -> io::Result<Self> {
        let username = read_string_with_len(reader, 2)?;
        Ok(Self::ClientLogin { username })
    }

    fn decode_client_update(mut reader: impl Read) -> io::Result<Self> {
        let recipient = read_string_with_len(&mut reader, 2)?;
        let message = read_string_with_len(reader, 5)?;
        Ok(Self::ClientUpdate { recipient, message })
    }

    fn decode_server_update(mut reader: impl Read) -> io::Result<Self> {
        let chat_content = read_string_with_len(&mut reader, 5)?;
        let recipient = read_string_with_len(&mut reader, 2)?;
        let all_users = read_string_with_len(reader, 5)?;
        let connected_users = all_users.split('&').map(String::from).collect();
        Ok(Self::ServerUpdate {
            chat_content,
            recipient,
            connected_users,
        })
    }
}

impl fmt::Display for Message {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        // SAFETY: Because `Self` is marked `repr(u8)`, its layout is a `repr(C)` `union`
        // between `repr(C)` structs, each of which has the `u8` discriminant as its first
        // field, so we can read the discriminant without offsetting the pointer.
        let discriminant = unsafe { *<*const _>::from(self).cast::<u8>() };
        write!(f, "{:03}", discriminant)?;
        match self {
            Message::ClientLogin { username } => write!(f, "{:02}{}", username.len(), username),

            Message::ClientUpdate { recipient, message } => {
                write!(
                    f,
                    "{:02}{}{:05}{}",
                    recipient.len(),
                    recipient,
                    message.len(),
                    message
                )
            }

            Message::ServerUpdate {
                chat_content,
                recipient,
                connected_users,
            } => {
                let all_users = connected_users.iter().fold(String::new(), |mut acc, user| {
                    if !acc.is_empty() {
                        acc.push('&')
                    }
                    acc.push_str(user);
                    acc
                });

                write!(
                    f,
                    "{:05}{}{:02}{}{:05}{}",
                    chat_content.len(),
                    chat_content,
                    recipient.len(),
                    recipient,
                    all_users.len(),
                    all_users
                )
            }

            _ => Ok(()),
        }
    }
}

fn read_string_with_len(mut reader: impl Read, len_len: usize) -> io::Result<String> {
    let mut buf = vec![0u8; len_len];
    reader.read_exact(&mut buf)?;
    let len = std::str::from_utf8(buf.as_slice())
        .map_err(io::Error::other)
        .and_then(|s| s.parse::<usize>().map_err(io::Error::other))?;

    if len == 0 {
        return Ok(String::new());
    }

    let mut data = vec![0u8; len];
    reader.read_exact(&mut data)?;
    String::from_utf8(data).map_err(io::Error::other)
}
