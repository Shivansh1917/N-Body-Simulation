import { useEffect, useState } from "react";

export default function Home() {
  const [messages, setMessages] = useState([]);

  useEffect(() => {
    const socket = new WebSocket("ws://localhost:8080");

    socket.onmessage = (event) => {
      const data = JSON.parse(event.data);
      setMessages((prev) => {
        console.log("Hi");
        console.log("New data received:", data);
        return [...prev, data];
      });
    };

    return () => socket.close();
  }, []);

  useEffect(() => {
    console.log("Updated messages:", messages);
  }, [messages]);

  return (
    <div>
      <h1>Live N-Body Data</h1>
      <ul>
        {messages.map((msg, index) => (
          <li key={index}>
            X: {msg.x}, Y: {msg.y}
          </li>
        ))}
      </ul>
    </div>
  );
}
