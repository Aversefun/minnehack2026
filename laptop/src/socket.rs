//! Websocket (de)serialization.

use ewebsock::{WsEvent, WsMessage};
use serde::de::Visitor;

#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum Button {
    Pause,
    Shop,
    A,
    D,
    FSpeed,
    Accel,
    Dura,
    End,
}

impl<'de> serde::Deserialize<'de> for Button {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        struct ButtonVisitor;
        impl<'de> Visitor<'de> for ButtonVisitor {
            type Value = Button;
            fn expecting(&self, formatter: &mut std::fmt::Formatter) -> std::fmt::Result {
                formatter.write_str("an integer between 0 and 7 mapped to a button")
            }
            // smaller types unnecessary, default implementation forwards
            fn visit_u64<E>(self, v: u64) -> Result<Self::Value, E>
            where
                E: serde::de::Error,
            {
                self.visit_u128(v as u128)
            }
            fn visit_u128<E>(self, v: u128) -> Result<Self::Value, E>
            where
                E: serde::de::Error,
            {
                Ok(match v {
                    0 => Button::Pause,
                    1 => Button::Shop,
                    2 => Button::A,
                    3 => Button::D,
                    4 => Button::FSpeed,
                    5 => Button::Accel,
                    6 => Button::Dura,
                    7 => Button::End,
                    _ => {
                        return Err(E::invalid_value(
                            serde::de::Unexpected::Unsigned(v as u64),
                            &self,
                        ));
                    }
                })
            }
        }
        deserializer.deserialize_u8(ButtonVisitor)
    }
}

#[derive(serde::Deserialize, Clone, Copy, Debug, PartialEq)]
#[serde(tag = "type")]
pub enum PacketToLaptop {
    #[serde(rename = "button_down")]
    ButtonPress { button: Button },
    #[serde(rename = "button_up")]
    ButtonRelease { button: Button },
    #[serde(rename = "gyro_update")]
    Gyro { x: f64, y: f64, z: f64 },
    #[serde(rename = "ping")]
    Ping,
}

#[derive(serde::Serialize, Clone, Copy, Debug, PartialEq, Eq, Hash)]
#[serde(tag = "type")]
pub enum PacketToPhone {
    #[serde(rename = "switch_home")]
    SwitchToHome,
    #[serde(rename = "switch_shop")]
    SwitchToShop,
    #[serde(rename = "ping")]
    Ping,
}

pub struct PhoneConnection {
    sender: ewebsock::WsSender,
    receiver: ewebsock::WsReceiver,
    closed: bool,
}

impl PhoneConnection {
    pub fn new() -> Self {
        let domain = web_sys::window()
            .unwrap()
            .document()
            .unwrap()
            .location()
            .unwrap()
            .host()
            .unwrap();

        let (sender, receiver) = ewebsock::connect(
            domain
                + &format!(
                    "/api/laptop_ws/{}",
                    web_sys::window()
                        .unwrap()
                        .prompt_with_message("lobby ID?")
                        .unwrap()
                        .unwrap()
                ),
            ewebsock::Options::default(),
        )
        .unwrap();

        Self {
            sender,
            receiver,
            closed: false,
        }
    }
    pub fn send(&mut self, packet: PacketToPhone) {
        self.sender.send(ewebsock::WsMessage::Text(
            serde_json::to_string(&packet).unwrap(),
        ));
    }
    pub fn recv(&mut self) -> Option<PacketToLaptop> {
        self.receiver.try_recv().and_then(|v| match v {
            WsEvent::Message(WsMessage::Text(s)) => Some(serde_json::from_str(&s).unwrap()),
            WsEvent::Closed => {
                self.closed = true;
                None
            }
            WsEvent::Opened => None,
            _ => panic!("non-message or non-text aaaa"),
        })
    }
    pub fn closed(&self) -> bool {
        self.closed
    }
}
