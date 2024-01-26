const MESSAGE_MAGIC: &str = "&MAGSH_MESSAGE&";
const AUTHOR_MAGIC: &str = "&Author&";
const DATA_MAGIC: &str = "&DATA&";

#[derive(Debug, Clone)]
pub struct ChatMessage {
    pub author: String,
    pub recipient: String,
    pub data: String,
}

impl ChatMessage {
    pub fn new(author: impl ToString, recipient: impl ToString, data: impl ToString) -> Self {
        Self {
            author: author.to_string(),
            recipient: recipient.to_string(),
            data: data.to_string(),
        }
    }
}

impl ToString for ChatMessage {
    fn to_string(&self) -> String {
        let mut output = String::new();
        output.push_str(MESSAGE_MAGIC);
        output.push_str(AUTHOR_MAGIC);
        output.push_str(&self.author);
        output.push_str(DATA_MAGIC);
        output.push_str(&self.data);
        output
    }
}
