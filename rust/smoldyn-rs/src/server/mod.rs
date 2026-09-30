//! server 

use axum::{extract::State, routing::get, Router};
use std::net::SocketAddr;
use std::sync::Arc;

pub (crate) struct AppState {
    app_name: String,
    version: String,
}

pub(crate) struct WebServer {
    addr: SocketAddr,
    state: Arc<AppState>,
}


impl WebServer {

    pub fn new(port: u16) -> Self {
        let addr = SocketAddr::from(([127, 0, 0, 1], port));
        let state = Arc::new(AppState {
            app_name: "Smoldyn".to_string(),
            version: env!("CARGO_PKG_VERSION").to_string(),
        });

        Self { addr, state }
    }

    pub async fn run(self) {
        // Build the router and attach the shared state
        let app = Router::new()
            .route("/", get(Self::root_handler))
            .with_state(self.state);

        // Bind the TCP listener
        let listener = tokio::net::TcpListener::bind(&self.addr)
            .await
            .unwrap();
        
        println!("Server running on http://{}", self.addr);

        // Start the server
        axum::serve(listener, app).await.unwrap();
    }

    // 4. Implement route handlers as associated functions using the State extractor
    async fn root_handler(State(state): State<Arc<AppState>>) -> String {
        format!("Welcome to {}!", state.app_name)
    }

}
